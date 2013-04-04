/* GStreamer demux bin
 * Copyright (C) 2013 CableLabs, Louisville, CO 80027
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "gstdemuxbin.h"

/**
 * SECTION:element-demuxbin
 *
 * DemuxBin provides a bin for demuxing MPEG streams
 *
 **/


/* pad templates */
static GstStaticPadTemplate demux_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate video_src_template =
    GST_STATIC_PAD_TEMPLATE ("video_%u",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate audio_src_template =
    GST_STATIC_PAD_TEMPLATE ("audio_%u",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate text_src_template =
    GST_STATIC_PAD_TEMPLATE ("text_%u",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate private_src_template =
    GST_STATIC_PAD_TEMPLATE ("private_%u",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS_ANY);

struct _GstDemuxBin
{
    /* Ghostpads with changing targets */
    GstPad* sinkpad;
    GstPad* vsrcpad;
    GstPad* asrcpad;
    GstPad* tsrcpad;
    GstPad* psrcpad;

    /* TRUE if in PAUSED/PLAYING */
    gboolean active;

    /* Increasing counter for unique pad name */
    guint last_audio_pad_id;
    guint last_video_pad_id;
    guint last_text_pad_id;

    /* Cached caps for identification */
    GstCaps* raw_video_caps;
    GstCaps* raw_audio_caps;
    GstCaps* raw_text_caps;
};

struct _GstDemuxBinClass
{
    /* Action Signals */
    GstPad* (*request_pad)(GstDemuxBin* demuxbin, GstCaps* caps);
};


#define DEFAULT_RAW_CAPS		\
  "video/x-raw; "				\
  "audio/x-raw; "				\
  "text/x-raw "

/* Properties */
enum
{
    PROP_0,
    PROP_LAST
};

/* Signals */
enum
{
    SIGNAL_0,
    LAST_SIGNAL
};

static guint gst_demux_bin_signals[LAST_SIGNAL] = { 0 };

static GstStaticCaps default_raw_caps = GST_STATIC_CAPS(DEFAULT_RAW_CAPS);

GST_DEBUG_CATEGORY_STATIC(gst_demux_bin_debug);
#define GST_CAT_DEFAULT gst_demux_bin_debug

G_DEFINE_TYPE(GstDemuxBin, gst_demux_bin, GST_TYPE_BIN);

static void gst_demux_bin_dispose(GObject* object);
static void gst_demux_bin_set_property(GObject* object, guint prop_id,
                                       const GValue* value, GParamSpec* pspec);
static void gst_demux_bin_get_property(GObject* object, guint prop_id,
                                       GValue* value, GParamSpec* pspec);
static GstStateChangeReturn gst_demux_bin_change_state(GstElement* element,
                                       GstStateChange transition);


static void
gst_demux_bin_class_init(GstDemuxBinClass* klass)
{
    GObjectClass *gobject_klass;
    GstElementClass *gstelement_klass;

    gobject_klass = (GObjectClass*)klass;
    gstelement_klass = (GstElementClass*)klass;

    gobject_klass->dispose = gst_demux_bin_dispose;
    gobject_klass->set_property = gst_demux_bin_set_property;
    gobject_klass->get_property = gst_demux_bin_get_property;

    /* Properties */


    /* Signals */


    /* Pads */
    gst_element_class_add_pad_template(gstelement_klass,
        gst_static_pad_template_get(&demux_sink_template));
    gst_element_class_add_pad_template(gstelement_klass,
        gst_static_pad_template_get(&video_src_template));
    gst_element_class_add_pad_template(gstelement_klass,
        gst_static_pad_template_get(&audio_src_template));
    gst_element_class_add_pad_template(gstelement_klass,
        gst_static_pad_template_get(&text_src_template));
    gst_element_class_add_pad_template(gstelement_klass,
        gst_static_pad_template_get(&private_src_template));

    gstelement_klass->change_state =
        GST_DEBUG_FUNCPTR(gst_demux_bin_change_state);

    gst_element_class_set_static_metadata(gstelement_klass,
        "Demux Bin",
        "Generic/Bin/Demuxer",
        "Convenience demuxing element",
        "<ruihri@cablelabs.com>");
}

static void
gst_demux_bin_dispose(GObject* object)
{
    GstDemuxBin *pBin = (GstDemuxBin*)object;

    if (pBin->raw_video_caps)
    {
        gst_caps_unref(pBin->raw_video_caps);
    }

    if (pBin->raw_audio_caps)
    {
        gst_caps_unref(pBin->raw_audio_caps);
    }

    if (pBin->raw_text_caps)
    {
        gst_caps_unref(pBin->raw_text_caps);
    }

    G_OBJECT_CLASS(gst_demux_bin_parent_class)->dispose(object);
}

static void
gst_demux_bin_init(GstDemuxBin* demux_bin)
{
    GstPadTemplate *tmp;

    demux_bin->last_audio_pad_id = 0;
    demux_bin->last_video_pad_id = 0;
    demux_bin->last_text_pad_id = 0;
    demux_bin->raw_video_caps = gst_caps_from_string ("video/x-raw");
    demux_bin->raw_audio_caps = gst_caps_from_string ("audio/x-raw");
    demux_bin->raw_text_caps = gst_caps_from_string ("text/x-raw");

    tmp = gst_static_pad_template_get(&demux_sink_template);
    demux_bin->sinkpad = gst_ghost_pad_new_no_target_from_template("sink", tmp);
    gst_object_unref(tmp);
    gst_element_add_pad(GST_ELEMENT_CAST(demux_bin), demux_bin->sinkpad);

    tmp = gst_static_pad_template_get(&video_src_template);
    demux_bin->vsrcpad =
               gst_ghost_pad_new_no_target_from_template("video_%u", tmp);
    gst_object_unref(tmp);
    gst_element_add_pad(GST_ELEMENT_CAST(demux_bin), demux_bin->vsrcpad);

    tmp = gst_static_pad_template_get(&audio_src_template);
    demux_bin->asrcpad =
               gst_ghost_pad_new_no_target_from_template("audio_%u", tmp);
    gst_object_unref(tmp);
    gst_element_add_pad(GST_ELEMENT_CAST(demux_bin), demux_bin->asrcpad);

    tmp = gst_static_pad_template_get(&text_src_template);
    demux_bin->tsrcpad =
               gst_ghost_pad_new_no_target_from_template("text_%u", tmp);
    gst_object_unref(tmp);
    gst_element_add_pad(GST_ELEMENT_CAST(demux_bin), demux_bin->tsrcpad);

    tmp = gst_static_pad_template_get(&private_src_template);
    demux_bin->psrcpad =
               gst_ghost_pad_new_no_target_from_template("private_%u", tmp);
    gst_object_unref(tmp);
    gst_element_add_pad(GST_ELEMENT_CAST(demux_bin), demux_bin->psrcpad);
}

static void
gst_demux_bin_set_property(GObject* object, guint prop_id,
                           const GValue* value, GParamSpec* pspec)
{
    switch (prop_id)
    {
      default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
          break;
    }
}

static void
gst_demux_bin_get_property(GObject* object, guint prop_id,
                           GValue* value, GParamSpec* pspec)
{
    switch (prop_id)
    {
      default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
          break;
    }
}

static gboolean
gst_demux_bin_activate(GstDemuxBin* pBin)
{
    pBin->active = TRUE;
    return pBin->active;
}

static void
gst_demux_bin_deactivate(GstDemuxBin* pBin)
{
    pBin->active = FALSE;
}

static GstStateChangeReturn
gst_demux_bin_change_state(GstElement* element, GstStateChange transition)
{
    GstStateChangeReturn ret;
    GstDemuxBin* pBin = (GstDemuxBin*)element;

    switch (transition)
    {
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            if (!gst_demux_bin_activate(pBin))
            {
                return GST_STATE_CHANGE_FAILURE;
            }
            break;
        default:
            break;
    }

    ret = GST_ELEMENT_CLASS(gst_demux_bin_parent_class)->change_state(element,
                                                                    transition);
    if (ret != GST_STATE_CHANGE_FAILURE)
    {
        switch (transition)
        {
            case GST_STATE_CHANGE_PAUSED_TO_READY:
                gst_demux_bin_deactivate(pBin);
                break;
            default:
                break;
        }
    }

    return ret;
}


static gboolean
plugin_init(GstPlugin* plugin)
{
    gboolean res;

    GST_DEBUG_CATEGORY_INIT(gst_demux_bin_debug, "demuxbin", 0, "demux bin");

    res = gst_element_register(plugin, "demuxbin",
                               GST_RANK_PRIMARY, GST_TYPE_DEMUX_BIN);
    return res;
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  demuxing,
                  "various demuxing-related elements",
                  plugin_init,
                  VERSION,
                  GST_LICENSE,
                  GST_PACKAGE_NAME,
                  GST_PACKAGE_ORIGIN)

