#ifndef _DRI_H

#define _DRI_H

#include <linux/device-mapper.h>

#define MODULE_NAME "dri"

#define LOG(fmt, ...) pr_info("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)
#define WRN(fmt, ...) pr_warn("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) pr_err("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)

static int dri_ctr(struct dm_target* ti, unsigned int argc, char* argv[]);

static void dri_dtr(struct dm_target* ti);

#endif // !_DRI_H
