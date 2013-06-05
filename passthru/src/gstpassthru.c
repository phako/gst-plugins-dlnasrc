/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2008 U-HATHORRyan <<user@hostname.org>>
 * Copyright (C) 2009 Cable Television Laboratories, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-passthru
 *
 * FIXME:Describe passthru here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! passthru ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <glib/gprintf.h>
#include <string.h>

#include "gstpassthru.h"

// Uncomment to compile under GStreamer-0.10 instead of GStreamer-1.0
//#define GSTREAMER_010

GST_DEBUG_CATEGORY_STATIC ( gst_pass_thru_debug);
#define /*lint -e(652)*/ GST_CAT_DEFAULT gst_pass_thru_debug

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0, PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
        GST_PAD_SINK, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
        GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));

#ifdef GSTREAMER_010
GST_BOILERPLATE (GstPassThru, gst_pass_thru, GstElement, GST_TYPE_ELEMENT)
#else
#define gst_pass_thru_parent_class parent_class
G_DEFINE_TYPE (GstPassThru, gst_pass_thru, GST_TYPE_ELEMENT);
#endif

static void gst_pass_thru_set_property (GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec);
static void gst_pass_thru_get_property(GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec);

#ifdef GSTREAMER_010
static gboolean gst_pass_thru_set_caps(GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_pass_thru_chain(GstPad * pad, GstBuffer * buf);
static gboolean gst_pass_thru_sink_event(GstPad *pad, GstEvent *event);
static gboolean gst_pass_thru_src_event(GstPad *pad, GstEvent *event);
#else
static gboolean gst_pass_thru_pad_event(GstPad *pad, GstObject *parent, GstEvent *event);
static GstFlowReturn gst_pass_thru_chain (GstPad * pad, GstObject* parent, GstBuffer * buf);
#endif

static GstStateChangeReturn gst_pass_thru_change_state(GstElement *element,
        GstStateChange transition);
/* GObject vmethod implementations */

#ifdef GSTREAMER_010
static void gst_pass_thru_base_init(gpointer gclass)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS(gclass);

    gst_element_class_set_details_simple(element_class, "PassThru",
            "FIXME:Generic", "FIXME:Generic Template Element",
            "U-HATHORRyan <<user@hostname.org>>");

    gst_element_class_add_pad_template(element_class,
            gst_static_pad_template_get(&src_factory));
    gst_element_class_add_pad_template(element_class,
            gst_static_pad_template_get(&sink_factory));
}
#endif

/* initialize the passthru's class */
static void gst_pass_thru_class_init(GstPassThruClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    GST_DEBUG_CATEGORY_INIT(gst_pass_thru_debug, "passthru", 0,
            "Template passthru");

    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;

    gobject_class->set_property = gst_pass_thru_set_property;
    gobject_class->get_property = gst_pass_thru_get_property;
    gstelement_class->change_state = gst_pass_thru_change_state;

#ifndef GSTREAMER_010
    gst_element_class_set_details_simple(gstelement_class,
			"Diagnostic pass thru",
			"Diagnostic", // see docs/design/draft-klass.txt
			"Used for diagnostic purposes",
			"U-Host <<user@hostname.org>> 3/28/13 11:45 AM");

	gst_element_class_add_pad_template (gstelement_class,
			gst_static_pad_template_get (&src_factory));
	gst_element_class_add_pad_template (gstelement_class,
			gst_static_pad_template_get (&sink_factory));
#endif

    g_object_class_install_property(gobject_class, PROP_SILENT,
            g_param_spec_boolean("silent", "Silent",
                    "Produce verbose output ?", FALSE, G_PARAM_READWRITE));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
#ifdef GSTREAMER_010
gst_pass_thru_init(GstPassThru * filter, GstPassThruClass * gclass)
#else
gst_pass_thru_init(GstPassThru * filter)
#endif
{
    filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
#ifdef GSTREAMER_010
    gst_pad_set_setcaps_function(filter->sinkpad, GST_DEBUG_FUNCPTR(
            gst_pass_thru_set_caps));
    gst_pad_set_getcaps_function(filter->sinkpad, GST_DEBUG_FUNCPTR(
            gst_pad_proxy_getcaps));
    gst_pad_set_event_function(filter->sinkpad, gst_pass_thru_sink_event);
#else
	GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
	gst_pad_set_event_function (filter->sinkpad,
				GST_DEBUG_FUNCPTR(gst_pass_thru_pad_event));
#endif

    gst_pad_set_chain_function(filter->sinkpad, GST_DEBUG_FUNCPTR(
            gst_pass_thru_chain));

    filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
#ifdef GSTREAMER_010
    gst_pad_set_getcaps_function(filter->srcpad, GST_DEBUG_FUNCPTR(
            gst_pad_proxy_getcaps));
    gst_pad_set_event_function(filter->srcpad, gst_pass_thru_src_event);
#else
	GST_PAD_SET_PROXY_CAPS (filter->srcpad);
#endif

    gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);
    gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);
    filter->silent = FALSE;

    //gst_debug_set_threshold_for_name("passthru", GST_LEVEL_INFO);
}

static void gst_pass_thru_set_property(GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec)
{
    GstPassThru *filter = GST_PASSTHRU(object);

    switch (prop_id)
    {
    case PROP_SILENT:
        filter->silent = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gst_pass_thru_get_property(GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec)
{
    GstPassThru *filter = GST_PASSTHRU(object);

    switch (prop_id)
    {
    case PROP_SILENT:
        g_value_set_boolean(value, filter->silent);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
#ifdef GSTREAMER_010
static gboolean gst_pass_thru_set_caps(GstPad * pad, GstCaps * caps)
{
    GstPassThru *filter;
    GstPad *otherpad;

    filter = GST_PASSTHRU(gst_pad_get_parent(pad));
    otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
    gst_object_unref(filter);

    return gst_pad_set_caps(otherpad, caps);
}
#endif

#ifndef GSTREAMER_010
static gboolean gst_pass_thru_pad_event(GstPad *pad, GstObject *parent, GstEvent *event)
{
	gboolean ret;
	GstPassThru *filter;
    filter = GST_PASSTHRU(parent);

	switch (GST_EVENT_TYPE (event))
	{
	case GST_EVENT_CAPS:
	{
		GstCaps * caps;

		gst_event_parse_caps (event, &caps);
		/* do something with the caps */
		GstPad *otherpad;

		otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;

		gst_pad_set_caps (otherpad, caps);

		/* and forward */
		ret = gst_pad_event_default (pad, parent, event);
		break;
	}
	default:
		ret = gst_pad_event_default (pad, parent, event);
		break;
	}
	return ret;
}
#endif

void gst_buffer_hexdump(GstObject *object, GstBuffer *buf)
{
#define DUMP_BYTES_PER_LINE 16
#define DUMP_CHARS_PER_BYTE  3
#define BYTE_LINE_LENGTH    (DUMP_BYTES_PER_LINE * DUMP_CHARS_PER_BYTE)

    guint index = 0;
    guint offset = 0;
    GstDebugLevel level;
#ifndef GSTREAMER_010
	GstMapInfo map;
	gst_buffer_map(buf, &map, GST_MAP_READ);
#endif

    gchar buffer[BYTE_LINE_LENGTH + 1];

    /* Batch up the level check, or else the filter is always too slow */
    level = gst_debug_category_get_threshold(gst_pass_thru_debug);
    if (level >= GST_LEVEL_DEBUG)
    {
        GST_DEBUG_OBJECT(object, "GstBuffer %p:", buf);

#ifdef GSTREAMER_010
        while (index < GST_BUFFER_SIZE(buf))
#else
        while (index < map.size)
#endif
        {
#ifdef GSTREAMER_010
            g_snprintf(&buffer[offset], DUMP_CHARS_PER_BYTE + 1, "%02X ",
                    GST_BUFFER_DATA(buf)[index]);
#else
            g_snprintf(&buffer[offset], DUMP_CHARS_PER_BYTE + 1, "%02X ",
                    map.data[index]);
#endif
            index++;
            offset = (offset + DUMP_CHARS_PER_BYTE) % BYTE_LINE_LENGTH;
            if (offset == 0)
            {
                GST_DEBUG_OBJECT(object, "%s", buffer);
            }
        }

        if (offset != 0)
        {
            buffer[offset] = '\0';
            GST_DEBUG_OBJECT(object, "%s", buffer);
        }
    }

#undef BYTE_LINE_LENGTH
#undef DUMP_CHARS_PER_BYTE
#undef DUMP_BYTES_PER_LINE
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
#ifdef GSTREAMER_010
gst_pass_thru_chain(GstPad * pad, GstBuffer * buf)
#else
gst_pass_thru_chain(GstPad * pad, GstObject * parent, GstBuffer * buf)
#endif
{
    GstPassThru *filter;
#ifndef GSTREAMER_010
	GstMapInfo map;
#endif

#ifdef GSTREAMER_010
	filter = GST_PASSTHRU(GST_OBJECT_PARENT(pad));
#else
	filter = GST_PASSTHRU(parent);
	gst_buffer_map (buf, &map, GST_MAP_READ);
#endif
    GST_ERROR_OBJECT(filter, "called");

    if (GST_BUFFER_TIMESTAMP_IS_VALID(buf))
    {
        GstClockTime buftime = GST_BUFFER_TIMESTAMP(buf);
        GST_ERROR_OBJECT(filter, "Buffer Timestamp = %" GST_TIME_FORMAT, GST_TIME_ARGS(buftime));
    }
    if (GST_BUFFER_DURATION_IS_VALID(buf))
    {
        GstClockTime buftime = GST_BUFFER_DURATION(buf);
        GST_LOG_OBJECT(filter, "Buffer duration = %" GST_TIME_FORMAT, GST_TIME_ARGS(buftime));

    }
    gst_buffer_hexdump(GST_OBJECT(filter), buf);

    /* just push out the incoming buffer without touching it */
    return gst_pad_push(filter->srcpad, buf);
}

#ifdef GSTREAMER_010
static gboolean gst_pass_thru_sink_event(GstPad *pad, GstEvent *event)
{
    GstPassThru *filter;

    filter = GST_PASSTHRU(GST_OBJECT_PARENT(pad));

    switch (GST_EVENT_TYPE(event))
    {
    case GST_EVENT_EOS:
        /* end-of-stream, we should close down all stream leftovers here */
        GST_INFO_OBJECT(filter, "Received EOS event");
        break;

    case GST_EVENT_NEWSEGMENT:
        GST_INFO_OBJECT(filter, "Received SEGMENT event");
        {
            gdouble rate;
            GstFormat format;
            gint64 start;
            gint64 stop;
            gint64 position;
            gboolean update;
            gst_event_parse_new_segment(event, &update, &rate, &format, &start,
                     &stop, &position);
            switch (format)
            {
            case GST_FORMAT_UNDEFINED:
                GST_INFO_OBJECT(filter, "FORMAT_UNDEFINED");
                break;
            case GST_FORMAT_DEFAULT:
                GST_INFO_OBJECT(filter, "FORMAT_DEFAULT");
                break;
            case GST_FORMAT_BYTES:
                GST_INFO_OBJECT(filter, "FORMAT_BYTES");
                break;
            case GST_FORMAT_TIME:
                GST_INFO_OBJECT(
                        filter,
                        "FORMAT_TIME, rate %3.1f, start %lld ns, stop %lld ns, pos %lld ns",
                        rate, start, stop, position);
                break;
            case GST_FORMAT_BUFFERS:
                GST_INFO_OBJECT(filter, "FORMAT_BUFFERS");
                break;
            case GST_FORMAT_PERCENT:
                GST_INFO_OBJECT(filter, "FORMAT_PERCENT");
                break;
            default:
                GST_INFO_OBJECT(filter, "unrecognized format (%d)", format);
                break;
            }

        }
        break;
    case GST_EVENT_FLUSH_START:
        GST_INFO_OBJECT(filter, "Received FLUSH_START event");
        break;
    case GST_EVENT_FLUSH_STOP:
        GST_INFO_OBJECT(filter, "Received FLUSH_STOP event");
        break;
    default:
        GST_LOG_OBJECT(filter, "Received other (0x%X) event.", GST_EVENT_TYPE(
                event));
        break;
    }

    return gst_pad_event_default(pad, event);
} /* gst_pass_thru_sink_event */
#endif

#ifdef GSTREAMER_010
static gboolean gst_pass_thru_src_event(GstPad *pad, GstEvent *event)
{
    GstPassThru *filter;

    filter = GST_PASSTHRU(GST_OBJECT_PARENT(pad));

    switch (GST_EVENT_TYPE(event))
    {
    case GST_EVENT_QOS:
        GST_LOG_OBJECT(filter, "Received QOS event");
        {
            gdouble proportion;
            GstClockTimeDiff diff;
            GstClockTime timestamp;
            gst_event_parse_qos(event, &proportion, &diff, &timestamp);
            GST_LOG_OBJECT(filter, "Proportion %f, diff %lld, time %llu",
                    proportion, diff, timestamp);

        }
        break;
    case GST_EVENT_SEEK:
        GST_INFO_OBJECT(filter, "Received SEEK event");
        break;
    case GST_EVENT_NAVIGATION:
        GST_INFO_OBJECT(filter, "Received NAVIGATION event");
        break;
    case GST_EVENT_LATENCY:
        GST_INFO_OBJECT(filter, "Received LATENCY event");
        break;
    case GST_EVENT_FLUSH_START:
        GST_INFO_OBJECT(filter, "Received FLUSH_START event");
        break;
    case GST_EVENT_FLUSH_STOP:
        GST_INFO_OBJECT(filter, "Received FLUSH_STOP event");
        break;
    default:
        GST_INFO_OBJECT(filter, "Received other (0x%X) event", GST_EVENT_TYPE(
                event));
        break;
    }

    return gst_pad_event_default(pad, event);
}
#endif

static GstStateChangeReturn gst_pass_thru_change_state(GstElement *element,
        GstStateChange transition)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    GstPassThru *filter = GST_PASSTHRU(element);
    gchar *str;

    switch (transition)
    {
    case GST_STATE_CHANGE_NULL_TO_READY:
        str = "NULL to READY";
        break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        str = "READY to PAUSED";
        break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        str = "PAUSED to PLAYING";
        break;
    default:
        str = NULL;
        break;
    }

    if (str)
    {
        GST_INFO_OBJECT(filter, "Received %s state change", str);
    }

    //    ret = (GST_ELEMENT_CLASS (filter->gclass->parent_class))->change_state (element, transition);
    //    ret = GST_ELEMENT_CLASS(filter->gclass->parent_class).change_state(element, transition);
    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        GST_INFO_OBJECT(filter, "State change in parent class failed");
        return ret;
    }

    switch (transition)
    {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        str = "PLAYING to PAUSED";
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        str = "PAUSED to READY";
        break;
    case GST_STATE_CHANGE_READY_TO_NULL:
        str = "READY to NULL";
        break;
    default:
        str = NULL;
        break;
    }

    if (str)
    {
        GST_INFO_OBJECT(filter, "Received %s state change", str);
    }

    return ret;
}

/*
 * The following section supports the GStreamer auto plugging infrastructure.
 * Set to 0 if this is done on a package level using (ie gstelements.[hc])
 */
#if 1

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
passthru_init (GstPlugin * passthru)
{
	/* debug category for fltering log messages
	 *
	 * exchange the string 'Template ' with your description
	 */
	GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "passthru",
			0, "Diagnostic pass thru");

	return gst_element_register (passthru, "passthru", GST_RANK_NONE,
			GST_TYPE_PASSTHRU);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "passthru"
#endif

/* gstreamer looks for this structure to register the plugin
 *
 * exchange the string 'Template' with your description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
#ifdef GSTREAMER_010
    "passthru",
#else
    passthru,
#endif
    "Diagnostic passthru plugin",
    (GstPluginInitFunc)passthru_init,
    VERSION,
    "LGPL", // should be "proprietary", but that gets us blacklisted
    "CableLabs RUI-H RI",
    "http://www.cablelabs.com/"
)

#endif

