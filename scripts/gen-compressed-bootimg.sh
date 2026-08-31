#!/bin/bash
#
# Copyright (c) 2026 Rockchip Electronics Co., Ltd
# SPDX-License-Identifier: GPL-2.0
#
# Unpack a FIT or Android boot image and generate compressed variants.
#

set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
UBOOT_ROOT=$(cd -- "${SCRIPT_DIR}/.." && pwd)
MKIMAGE=${MKIMAGE:-${UBOOT_ROOT}/tools/mkimage}
FIT_UNPACK=${FIT_UNPACK:-${SCRIPT_DIR}/fit-unpack.sh}
ANDROID_UNPACK=${ANDROID_UNPACK:-${SCRIPT_DIR}/unpack_bootimg}
ANDROID_REPACK=${ANDROID_REPACK:-${SCRIPT_DIR}/repack-bootimg}

usage()
{
	echo "Usage: $0 -f [boot.img/fit/itb] -o [output]" >&2
}

INPUT_IMAGE=
OUTPUT_DIR=
while (($#)); do
	case $1 in
	-f)
		(($# >= 2)) || { usage; exit 1; }
		INPUT_IMAGE=$2
		shift 2
		;;
	-o)
		(($# >= 2)) || { usage; exit 1; }
		OUTPUT_DIR=$2
		shift 2
		;;
	-h|--help)
		usage
		exit 0
		;;
	*)
		usage
		exit 1
		;;
	esac
done

if [[ -z ${INPUT_IMAGE} || -z ${OUTPUT_DIR} ]]; then
	usage
	exit 1
fi
[[ -f ${INPUT_IMAGE} ]] || { echo "ERROR: No boot image: ${INPUT_IMAGE}" >&2; exit 1; }

INPUT_IMAGE=$(cd -- "$(dirname -- "${INPUT_IMAGE}")" && pwd)/$(basename -- "${INPUT_IMAGE}")
image_info=$(file -b -- "${INPUT_IMAGE}")
case ${image_info} in
*"Android bootimg"*)
	image_format=android
	image_type="Android boot"
	[[ -x ${ANDROID_UNPACK} ]] || { echo "ERROR: unpack_bootimg is not executable: ${ANDROID_UNPACK}" >&2; exit 1; }
	[[ -x ${ANDROID_REPACK} ]] || { echo "ERROR: repack-bootimg is not executable: ${ANDROID_REPACK}" >&2; exit 1; }
	;;
*"Device Tree Blob"*)
	image_format=fit
	image_type=FIT
	[[ -x ${FIT_UNPACK} ]] || { echo "ERROR: fit-unpack.sh is not executable: ${FIT_UNPACK}" >&2; exit 1; }
	[[ -x ${MKIMAGE} ]] || { echo "ERROR: mkimage is not executable: ${MKIMAGE}" >&2; exit 1; }
	command -v fdtget >/dev/null 2>&1 || { echo "ERROR: fdtget is not available" >&2; exit 1; }
	fdtget -l "${INPUT_IMAGE}" /images >/dev/null 2>&1 || {
		echo "ERROR: Input is not a FIT image: ${INPUT_IMAGE}" >&2
		exit 1
	}
	;;
*)
	echo "ERROR: Unsupported boot image format: ${image_info}" >&2
	exit 1
	;;
esac
echo "### Detected input format: ${image_type} image"

mkdir -p -- "${OUTPUT_DIR}"
OUTPUT_DIR=$(cd -- "${OUTPUT_DIR}" && pwd)
work_dir=$(mktemp -d "${SCRIPT_DIR}/.gen-compressed-bootimg.XXXXXX")
trap 'rm -rf -- "${work_dir}"' EXIT

if [[ ${image_format} == fit ]]; then
	echo "### Unpacking ${image_type} image: ${INPUT_IMAGE}"
	"${FIT_UNPACK}" -f "${INPUT_IMAGE}" -o "${OUTPUT_DIR}"
	[[ -f ${OUTPUT_DIR}/image.its && -f ${OUTPUT_DIR}/kernel ]] || {
		echo "ERROR: fit-unpack.sh did not produce image.its and kernel in ${OUTPUT_DIR}" >&2
		exit 1
	}
else
	android_dir=${work_dir}/android
	echo "### Unpacking ${image_type} image: ${INPUT_IMAGE}"
	"${ANDROID_UNPACK}" --boot_img "${INPUT_IMAGE}" --out "${android_dir}"
	[[ -f ${android_dir}/kernel ]] || {
		echo "ERROR: unpack_bootimg did not produce kernel" >&2
		exit 1
	}
fi

for compression in gzip lzma lz4; do
	variant_dir=${work_dir}/${compression}
	output_image=${OUTPUT_DIR}/boot-${compression}.img
	echo "### Generating ${compression}-compressed ${image_type} image: ${output_image}"
	mkdir -p -- "${variant_dir}"
	if [[ ${image_format} == fit ]]; then
		cp -a -- "${OUTPUT_DIR}/." "${variant_dir}/"
	else
		cp -- "${android_dir}/kernel" "${variant_dir}/kernel"
	fi

	case ${compression} in
	gzip)
		gzip -c -9 -- "${variant_dir}/kernel" > "${variant_dir}/kernel.gz"
		suffix=gz
		;;
	lzma|lz4)
		"${UBOOT_ROOT}/scripts/compress.sh" "${compression}" "${variant_dir}/kernel"
		suffix=${compression}
		;;
	esac

	if [[ ${image_format} == fit ]]; then
		# Change only the kernel data file and compression in the unpacked ITS.
		perl -0pi -e \
			's{(kernel\s*\{.*?data\s*=\s*/incbin/\(")kernel("\);.*?compression\s*=\s*)"none"}{$1kernel.'"${suffix}"'$2"'"${compression}"'"}s' \
			"${variant_dir}/image.its"

		grep -Fq "kernel.${suffix}" "${variant_dir}/image.its" || {
			echo "ERROR: failed to replace FIT kernel data in ${variant_dir}/image.its" >&2
			exit 1
		}
		grep -Fq "compression = \"${compression}\"" "${variant_dir}/image.its" || {
			echo "ERROR: failed to replace FIT kernel compression in ${variant_dir}/image.its" >&2
			exit 1
		}

		( cd -- "${variant_dir}" && "${MKIMAGE}" -f image.its -E -p 0x1200 "${output_image}" )
	else
		srctree=${UBOOT_ROOT} objtree=${UBOOT_ROOT} "${ANDROID_REPACK}" \
			--boot_img "${INPUT_IMAGE}" \
			--out "${variant_dir}/repack" \
			--kernel "${variant_dir}/kernel.${suffix}" \
			-o "${output_image}"
	fi
	echo "### Generated: ${output_image}"
done

echo "### Generated all compressed boot images in: ${OUTPUT_DIR}"
