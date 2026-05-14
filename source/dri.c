#include "dri.h"

#include <asm-generic/errno-base.h>
#include <linux/mm.h>

static struct target_type dri_target = {
  .name     = "dri",
  .version  = {1, 0, 0},
  .features = DM_TARGET_NOWAIT,
  .module   = THIS_MODULE,
  .ctr      = dri_ctr,
  .map      = dri_map,
  .dtr      = dri_dtr,
};

static int dri_ctr(struct dm_target* ti, unsigned int argc, char* argv[]) {
  int ret = 0;
  struct dri_dev_ctx* dev_ctx = NULL;
  LOG("ctr called");

  dev_ctx = (struct dri_dev_ctx*) kvmalloc(sizeof(*dev_ctx), GFP_KERNEL);
  if (dev_ctx == NULL) {
    ti->error = "could not alloc mem for dev_ctx";
    ret = -ENOMEM;
    goto end;
  }

  dev_ctx->intervals = RB_ROOT_CACHED;
  spin_lock_init(&dev_ctx->lock);
  ti->private = dev_ctx;

  LOG("path: %s", argv[0]);

  ret = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &dev_ctx->ddev);
  if (ret) {
    ti->error = "could not get device";
    ret = -ENOENT;
    goto end;
  }

end:
  if (dev_ctx && ret) {
    kvfree(dev_ctx);
    dev_ctx = NULL;
    ti->private = NULL;
  }
  return ret;
}

static void dri_end_io(struct bio* cbio) {
  struct dri_clone* dc = cbio->bi_private;
  struct dri_dev_ctx* ctx = dc->ctx;
  unsigned long flags;

  spin_lock_irqsave(&ctx->lock, flags);
  interval_tree_remove(&dc->io.node, &ctx->intervals);
  spin_unlock_irqrestore(&ctx->lock, flags);

  struct bio* orig = dc->bio;
  orig->bi_status = cbio->bi_status;
  bio_endio(orig);

  bio_put(cbio);
  kvfree(dc);
}

static int dri_map(struct dm_target* ti, struct bio* bio) {
  struct dri_dev_ctx* ctx = (struct dri_dev_ctx*)ti->private;
  sector_t start = bio->bi_iter.bi_sector;
  sector_t size_sectors = bio->bi_iter.bi_size >> SECTOR_SHIFT;
  sector_t last = start + size_sectors - 1;
  enum req_op op = bio_op(bio);
  unsigned long flags;

  spin_lock_irqsave(&ctx->lock, flags);
  struct interval_tree_node* node = interval_tree_iter_first(&ctx->intervals, start, last);
  while (node) {
    struct dri_io* io = container_of(node, struct dri_io, node);
    if ((op == REQ_OP_WRITE) ||
      (op == REQ_OP_READ && io->req_type == REQ_OP_WRITE)) {
      WRN("data race, sectors [%llu, %llu]", start, last);
      break;
    }
    node = interval_tree_iter_next(node, start, last);
  }

  struct dri_clone* clone = (struct dri_clone*) kvmalloc(sizeof(*clone), GFP_NOIO);
  if (clone == NULL) {
    spin_unlock_irqrestore(&ctx->lock, flags);
    return -EIO;
  }
  clone->bio = bio;
  clone->io.node.start = start;
  clone->io.node.last = last;
  clone->io.req_type = op;
  clone->ctx = ctx;
  interval_tree_insert(&clone->io.node, &ctx->intervals);

  spin_unlock_irqrestore(&ctx->lock, flags);

  struct bio* bio_clone = bio_alloc_clone(bio->bi_bdev, bio, GFP_NOIO, &fs_bio_set);
  if (bio_clone == NULL) {
    spin_lock_irqsave(&ctx->lock, flags);
    interval_tree_remove(&clone->io.node, &ctx->intervals);
    spin_unlock_irqrestore(&ctx->lock, flags);
    kvfree(clone);
    return -EIO;
  }

  bio_set_dev(bio_clone, ctx->ddev->bdev);
  bio_clone->bi_private = clone;
  bio_clone->bi_end_io = dri_end_io;

  submit_bio_noacct(bio_clone);
  return DM_MAPIO_SUBMITTED;
}

static void dri_dtr(struct dm_target* ti) {
  LOG("dtr called");
  struct dri_dev_ctx* ctx = (struct dri_dev_ctx*) ti->private;

  dm_put_device(ti, ctx->ddev);
  if (ti->private)
    kvfree(ti->private);
  ti->private = NULL;
}

static int __init dri_init(void) {
  LOG("init");
  dm_register_target(&dri_target);
  LOG("registered dri_target");
  return 0;
}

static void __exit dri_exit(void) {
  LOG("exit");
  dm_unregister_target(&dri_target);
}

module_init(dri_init);
module_exit(dri_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("deadxraver");
MODULE_DESCRIPTION("Data Race Inspector");

