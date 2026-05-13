#include "dri.h"

static struct target_type dri_target = {
  .name     = "dri",
  .version  = {1, 0, 0},
  .features = DM_TARGET_NOWAIT,
  .module   = THIS_MODULE,
  .ctr      = dri_ctr,
  .dtr      = dri_dtr,
};

static int dri_ctr(struct dm_target* ti, unsigned int argc, char* argv[]) {
  LOG("ctr called");
  return 0;
}

static void dri_dtr(struct dm_target* ti) {
  LOG("dtr called");
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
MODULE_DESCRIPTION("Device Mapper Proxy");

