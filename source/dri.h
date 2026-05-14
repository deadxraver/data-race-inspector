#ifndef _DRI_H

#define _DRI_H

#include <linux/blk_types.h>
#include <linux/interval_tree.h>
#include <linux/device-mapper.h>

#define MODULE_NAME "dri"

#define LOG(fmt, ...) pr_info("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)
#define WRN(fmt, ...) pr_warn("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) pr_err("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)

struct dri_io {
  struct interval_tree_node node;
  enum req_op req_type;
};

struct dri_dev_ctx {
  struct dm_dev* ddev;
  spinlock_t lock;
  struct rb_root_cached intervals;
};

struct dri_clone {
  struct bio* bio;
  struct dri_io io;
  struct dri_dev_ctx* ctx;
};

static int dri_ctr(struct dm_target* ti, unsigned int argc, char* argv[]);

static int dri_map(struct dm_target* ti, struct bio* bio);

static void dri_dtr(struct dm_target* ti);

#endif // !_DRI_H
