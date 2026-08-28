#!/bin/bash
#
# Copyright (c) 2026 Rockchip Electronics Co., Ltd
#
# SPDX-License-Identifier: GPL-2.0
#

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
AVBTOOL=${SCRIPT_DIR}/avbtool.py
CERT_GENERATOR=${SCRIPT_DIR}/avb_cert_generate_test_data
UNPACK_BOOTIMG=${SCRIPT_DIR}/../unpack_bootimg

usage() {
  cat <<EOF
Usage: $(basename "$0") -i <INPUT_DIR> -o <OUTPUT_DIR>

Options:
  -i INPUT_DIR   Directory containing unsigned images and parameter.txt.
  -o OUTPUT_DIR  Directory for signed images, certificate files, and vbmeta.img.
  -h             Show this help message.
EOF
}

info() {
  printf '\n### %s\n' "$*"
}

die() {
  echo "Error: avb_sign_image.sh: $*" >&2
  exit 1
}

warn() {
  printf '### WARNING: %s!!!\n' "$*" >&2
}

input_dir=
output_dir=
input_set=0
output_set=0
while getopts ':i:o:h' opt; do
  case ${opt} in
    i) input_dir=$OPTARG; input_set=1 ;;
    o) output_dir=$OPTARG; output_set=1 ;;
    h) usage; exit 0 ;;
    :) usage >&2; die "option -${OPTARG} requires an argument" ;;
    \?) usage >&2; die "unknown option: -${OPTARG}" ;;
  esac
done
shift $((OPTIND - 1))
(( input_set == 1 && output_set == 1 )) || {
  usage >&2
  die "both -i input-dir and -o output-dir are required"
}
(( $# == 0 )) || {
  usage >&2
  die "unexpected argument: $1"
}

[[ -d ${input_dir} ]] || die "missing input directory: ${input_dir}"
input_dir=$(cd "${input_dir}" && pwd)
mkdir -p "${output_dir}"
output_dir=$(cd "${output_dir}" && pwd)

IMAGE_DIR=${output_dir}
PARAMETER_FILE=${input_dir}/parameter.txt
KEY=${IMAGE_DIR}/testkey_cert_psk.pem
METADATA=${IMAGE_DIR}/cert_metadata.bin

info "Checking AVB build inputs"
printf '    input directory:  %s\n' "${input_dir}"
printf '    output directory: %s\n' "${output_dir}"
[[ -f ${PARAMETER_FILE} ]] || die "missing partition table: ${PARAMETER_FILE}"
[[ -x ${AVBTOOL} ]] || die "avbtool.py is not executable: ${AVBTOOL}"
[[ -x ${CERT_GENERATOR} ]] || die "certificate generator is not executable: ${CERT_GENERATOR}"
[[ -x ${UNPACK_BOOTIMG} ]] || die "unpack_bootimg is not executable: ${UNPACK_BOOTIMG}"

BOOT_IMAGE_TMPDIR=$(mktemp -d)
cleanup() {
  find "${BOOT_IMAGE_TMPDIR}" -depth -delete
}
trap cleanup EXIT

if [[ ${input_dir} != "${output_dir}" ]]; then
  info "Copying input images to ${output_dir}"
  cp -f "${PARAMETER_FILE}" "${IMAGE_DIR}/parameter.txt"
  shopt -s nullglob
  input_images=("${input_dir}"/*.img)
  ((${#input_images[@]} > 0)) || die "no image files found in ${input_dir}"
  cp -f "${input_images[@]}" "${IMAGE_DIR}/"
  shopt -u nullglob
fi

# The certificate generator writes its output in the current directory.
if [[ ! -f ${KEY} || ! -f ${METADATA} ]]; then
  info "Generating AVB certificate materials"
  (cd "${IMAGE_DIR}" && "${CERT_GENERATOR}")
else
  info "Using existing AVB certificate materials"
fi
[[ -f ${KEY} ]] || die "certificate key was not generated: ${KEY}"
[[ -f ${METADATA} ]] || die "certificate metadata was not generated: ${METADATA}"

info "Reading partition sizes from ${PARAMETER_FILE}"
cmdline=$(awk -F'CMDLINE:' '/^CMDLINE:/{print $2; exit}' "${PARAMETER_FILE}")
[[ -n ${cmdline} ]] || die "CMDLINE entry not found in ${PARAMETER_FILE}"
partitions=${cmdline#*mtdparts=}
[[ ${partitions} != "${cmdline}" ]] || die "mtdparts entry not found in ${PARAMETER_FILE}"

partition_size() {
  local name=$1 entry blocks
  while IFS= read -r entry; do
    if [[ ${entry} =~ ^([0-9A-Fa-fxX]+)@[^\(]+\(${name}\) ]]; then
      blocks=${BASH_REMATCH[1]}
      [[ ${blocks} != "-" ]] || return 1
      blocks=${blocks#0x}
      blocks=${blocks#0X}
      printf '%d\n' $((16#${blocks} * 512))
      return 0
    fi
  done < <(tr ',' '\n' <<< "${partitions}")
  return 1
}

partition_exists() {
  local name=$1 entry
  while IFS= read -r entry; do
    if [[ ${entry} =~ ^[^,]+@[^\(]+\(${name}\) ]]; then
      return 0
    fi
  done < <(tr ',' '\n' <<< "${partitions}")
  return 1
}

info "Checking input images against the partition table"
shopt -s nullglob
input_images=("${input_dir}"/*.img)
for image in "${input_images[@]}"; do
  image_name=$(basename "${image}" .img)
  partition_exists "${image_name}" || warn "${image_name}.img is not listed in ${PARAMETER_FILE}"
done
shopt -u nullglob

add_footer() {
  local name=$1 image=${IMAGE_DIR}/$1.img size
  [[ -f ${image} ]] || return 1
  size=$(partition_size "${name}") || die "partition '${name}' not found or has no fixed size"
  info "Adding AVB hash footer to ${name}.img (partition size: ${size} bytes)"

  local args=("${AVBTOOL}" add_hash_footer
    --image "${image}" --partition_name "${name}" --partition_size "${size}"
    --hash_algorithm sha256 --algorithm SHA256_RSA4096 --key "${KEY}")
  if [[ ${name} == boot || ${name} == recovery ]]; then
    local unpack_dir=${BOOT_IMAGE_TMPDIR}/${name}
    local unpack_output os_version
    mkdir -p "${unpack_dir}"
    unpack_output=$("${UNPACK_BOOTIMG}" --boot_img "${image}" --out "${unpack_dir}" --format info) ||
      die "failed to parse os version from ${name}.img"
    os_version=$(awk -F': ' '$1 == "os version" { print $2; exit }' <<< "${unpack_output}")
    [[ ${os_version} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] ||
      die "invalid os version '${os_version}' in ${name}.img"
    info "Using Android OS version ${os_version} for ${name}.img"
    args+=(--prop "com.android.build.boot.os_version:${os_version}")
  fi
  if [[ ${name} == recovery ]]; then
    args+=(--public_key_metadata "${METADATA}")
  fi
  "${args[@]}"
  return 0
}

descriptor_images=()
for name in dtbo boot recovery; do
  if add_footer "${name}"; then
    descriptor_images+=("${IMAGE_DIR}/${name}.img")
  else
    info "Skipping ${name}.img (file not found)"
  fi
done
(( ${#descriptor_images[@]} > 0 )) || die "no signable images found in ${IMAGE_DIR}"

info "Generating ${IMAGE_DIR}/vbmeta.img"
vbmeta_args=("${AVBTOOL}" make_vbmeta_image
  --public_key_metadata "${METADATA}" --algorithm SHA256_RSA4096
  --key "${KEY}" --padding_size 4096 --output "${IMAGE_DIR}/vbmeta.img")
for image in "${descriptor_images[@]}"; do
  vbmeta_args+=(--include_descriptors_from_image "${image}")
done
"${vbmeta_args[@]}"

echo "Generated ${IMAGE_DIR}/vbmeta.img"
