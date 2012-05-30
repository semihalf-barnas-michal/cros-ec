/* Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "config.h"
#include "comm-host.h"
#include "ec_commands.h"
#include "system.h"

#define STR0(name)  #name
#define STR(name)   STR0(name)

static const char * const part_name[] = {"unknown", "RO", "A", "B"};

enum ec_current_image get_version(void)
{
	struct ec_response_get_version r;
	struct ec_response_get_build_info r2;
	int res;

	res = ec_command(EC_CMD_GET_VERSION, NULL, 0, &r, sizeof(r));
	if (res)
		return res;
	res = ec_command(EC_CMD_GET_BUILD_INFO,
			NULL, 0, &r2, sizeof(r2));
	if (res)
		return res;

	/* Ensure versions are null-terminated before we print them */
	r.version_string_ro[sizeof(r.version_string_ro) - 1] = '\0';
	r.version_string_rw_a[sizeof(r.version_string_rw_a) - 1] = '\0';
	r.version_string_rw_b[sizeof(r.version_string_rw_b) - 1] = '\0';
	r2.build_string[sizeof(r2.build_string) - 1] = '\0';

	/* Print versions */
	printf("RO version:    %s\n", r.version_string_ro);
	printf("RW-A version:  %s\n", r.version_string_rw_a);
	printf("RW-B version:  %s\n", r.version_string_rw_b);
	printf("Firmware copy: %s\n",
	       (r.current_image < sizeof(part_name)/sizeof(part_name[0]) ?
		part_name[r.current_image] : "?"));
	printf("Build info:    %s\n", r2.build_string);

	return r.current_image;
}

int flash_partition(enum ec_current_image part, const uint8_t *payload,
		    uint32_t offset, uint32_t size)
{
	struct ec_params_reboot_ec rst_req;
	struct ec_params_flash_erase er_req;
	struct ec_params_flash_write wr_req;
	struct ec_params_flash_read rd_req;
	struct ec_response_flash_read rd_resp;
	int res;
	uint32_t i;
	enum ec_current_image current;

	current = get_version();
	if (current == part) {
		rst_req.target = part == EC_IMAGE_RO ?
				EC_IMAGE_RW_A : EC_IMAGE_RO;
		ec_command(EC_CMD_REBOOT_EC, &rst_req, sizeof(rst_req),
			   NULL, 0);
		/* wait EC reboot */
		usleep(500000);
	}

	printf("Erasing partition %s : 0x%x bytes at 0x%08x\n",
	       part_name[part], size, offset);
	er_req.size = size;
	er_req.offset = offset;
	res = ec_command(EC_CMD_FLASH_ERASE, &er_req, sizeof(er_req), NULL, 0);
	if (res) {
		fprintf(stderr, "Erase failed : %d\n", res);
		return -1;
	}

	printf("Writing partition %s : 0x%x bytes at 0x%08x\n",
	       part_name[part], size, offset);
	/* Write data in chunks */
	for (i = 0; i < size; i += EC_FLASH_SIZE_MAX) {
		wr_req.offset = offset + i;
		wr_req.size = MIN(size - i, EC_FLASH_SIZE_MAX);
		memcpy(wr_req.data, payload + i, wr_req.size);
		res = ec_command(EC_CMD_FLASH_WRITE, &wr_req, sizeof(wr_req),
				 NULL, 0);
		if (res) {
			fprintf(stderr, "Write error at 0x%08x : %d\n", i, res);
			return -1;
		}
	}

	printf("Verifying partition %s : 0x%x bytes at 0x%08x\n",
	       part_name[part], size, offset);
	/* Read data in chunks */
	for (i = 0; i < size; i += EC_FLASH_SIZE_MAX) {
		rd_req.offset = offset + i;
		rd_req.size = MIN(size - i, EC_FLASH_SIZE_MAX);
		res = ec_command(EC_CMD_FLASH_READ, &rd_req, sizeof(rd_req),
				 &rd_resp, sizeof(rd_resp));
		if (res) {
			fprintf(stderr, "Read error at 0x%08x : %d\n", i, res);
			return -1;
		}
		if (memcmp(payload + i, rd_resp.data, rd_req.size))
			fprintf(stderr, "ERR: @%08x->%08x\n",
				offset + i, offset + i + size);
	}
	printf("Done.\n");
	get_version();

	return 0;
}

/* black magic to include the EC firmware binary as a payload inside
 * the flashing program without relying on external tools such as objcopy or a
 * binary to header converter. */
__asm__ (".section .payload, \"ax\"\n"
	 "_payload_start:\n"
	 ".incbin \""STR(OUTDIR)"/"STR(PROJECT)".bin\"\n");

int main(int argc, char *argv[])
{
	extern uint8_t data[] asm("_payload_start");

	if (comm_init() < 0)
		return -3;

#ifndef CONFIG_NO_RW_B
	flash_partition(EC_IMAGE_RW_B, data + CONFIG_FW_B_OFF,
			CONFIG_FW_B_OFF, CONFIG_FW_B_SIZE);
#endif /* !CONFIG_NO_RW_B */
	flash_partition(EC_IMAGE_RW_A, data + CONFIG_FW_A_OFF,
			CONFIG_FW_A_OFF, CONFIG_FW_A_SIZE);
	flash_partition(EC_IMAGE_RO, data + CONFIG_FW_RO_OFF,
			CONFIG_FW_RO_OFF, CONFIG_FW_RO_SIZE);

	return 0;
}
