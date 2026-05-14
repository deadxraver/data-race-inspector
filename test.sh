SIZE=2097152

DEV=/dev/mapper/zero1

dmsetup create zero1 --table "0 $SIZE zero"

dmsetup create my0 --table "0 $SIZE dri $DEV"

dd oflag=direct if=/dev/urandom of=/dev/mapper/my0 \
  bs=32k count=1 seek=4 &

dd oflag=direct if=/dev/urandom of=/dev/mapper/my0 \
  bs=8k count=1 seek=17

sleep 2s

dmsetup remove my0
dmsetup remove zero1
