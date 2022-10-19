
ABI := armeabi-v7a

all:
	adb push ./libs/$(ABI)/test /data/local/tmp/test
	adb shell chmod 777 /data/local/tmp/test
	adb shell /data/local/tmp/test