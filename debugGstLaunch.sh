#!/bin/sh
rm *.txt
GST_PLUGIN_PATH=/usr/local/lib/gstreamer-0.10
gdb --command=debugGstLaunch_gdb.cmd --args gst-launch \
	--gst-debug-level=1 --gst-mm=0.10 --gst-debug=dlnasrc:3\
        --gst-plugin-load=$GST_PLUGIN_PATH/libgstdlnasrc.so \
	-v dlnasrc uri=http://192.168.2.2:8008/ocaphn/recording?rrid=15\&profile=DTCP_MPEG_TS_SD_NA_ISO\&mime=video/mpeg 
#	-v dlnasrc uri=http://192.168.2.3:8008/ocaphn/recording?rrid=1\&profile=MPEG_TS_SD_NA_ISO\&mime=video/mpeg 