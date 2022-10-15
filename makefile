

all:
	adb push ./libs/armeabi-v7a/test /data/local/tmp/test
	adb shell chmod 777 /data/local/tmp/test
	adb shell /data/local/tmp/test