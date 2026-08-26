#!/bin/bash
#
# Copyright (c) 2026 Rockchip Electronics Co., Ltd
# SPDX-License-Identifier: GPL-2.0
#
# Unpack a FIT image and generate gzip, lzma, and lz4 variants.
#

set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
UBOOT_ROOT=$(cd -- "${SCRIPT_DIR}/.." && pwd)
MKIMAGE=${MKIMAGE:-${UBOOT_ROOT}/tools/mkimage}
FIT_UNPACK=${FIT_UNPACK:-${SCRIPT_DIR}/fit-unpack.sh}

usage()
{
	echo "Usage: $0 -f [fit/itb] -o [output]" >&2
}

INPUT_FIT_FILE=
OUTPUT_DIR=
while (($#)); do
	case $1 in
	-f)
		(($# >= 2)) || { usage; exit 1; }
		INPUT_FIT_FILE=$2
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

if [[ -z ${INPUT_FIT_FILE} || -z ${OUTPUT_DIR} ]]; then
	usage
	exit 1
fi
[[ -f ${INPUT_FIT_FILE} ]] || { echo "ERROR: No FIT file: ${INPUT_FIT_FILE}" >&2; exit 1; }
[[ -x ${FIT_UNPACK} ]] || { echo "ERROR: fit-unpack.sh is not executable: ${FIT_UNPACK}" >&2; exit 1; }
[[ -x ${MKIMAGE} ]] || { echo "ERROR: mkimage is not executable: ${MKIMAGE}" >&2; exit 1; }

mkdir -p -- "${OUTPUT_DIR}"
OUTPUT_DIR=$(cd -- "${OUTPUT_DIR}" && pwd)
"${FIT_UNPACK}" -f "${INPUT_FIT_FILE}" -o "${OUTPUT_DIR}"
[[ -f ${OUTPUT_DIR}/image.its && -f ${OUTPUT_DIR}/kernel ]] || {
	echo "ERROR: fit-unpack.sh did not produce image.its and kernel in ${OUTPUT_DIR}" >&2
	exit 1
}

work_dir=$(mktemp -d "${SCRIPT_DIR}/.fit-gen-compressed.XXXXXX")
trap 'rm -rf -- "${work_dir}"' EXIT

for compression in gzip lzma lz4; do
	variant_dir=${work_dir}/${compression}
	mkdir -p -- "${variant_dir}"
	cp -a -- "${OUTPUT_DIR}/." "${variant_dir}/"

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

	# Change only the kernel data file and compression in the unpacked ITS.
	perl -0pi -e \
		's{(kernel\s*\{.*?data\s*=\s*/incbin/\(")kernel("\);.*?compression\s*=\s*)"none"}{$1kernel.'"${suffix}"'$2"'"${compression}"'"}s' \
		"${variant_dir}/image.its"

	( cd -- "${variant_dir}" && "${MKIMAGE}" -f image.its -E -p 0x1200 "${OUTPUT_DIR}/boot-${compression}.img" )
done

echo "Generated: ${OUTPUT_DIR}/boot-gzip.img ${OUTPUT_DIR}/boot-lzma.img ${OUTPUT_DIR}/boot-lz4.img"
