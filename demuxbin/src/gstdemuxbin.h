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

#ifndef __GST_DEMUXBIN_H__
#define __GST_DEMUXBIN_H__

#include <gst/gst.h>

#define GST_TYPE_DEMUX_BIN               (gst_demux_bin_get_type())
#define GST_DEMUX_BIN(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DEMUX_BIN,GstDemuxBin))
#define GST_DEMUX_BIN_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DEMUX_BIN,GstDemuxBinClass))
#define GST_IS_DEMUX_BIN(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DEMUX_BIN))
#define GST_IS_DEMUX_BIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DEMUX_BIN))

typedef struct _GstDemuxBin GstDemuxBin;
typedef struct _GstDemuxBinClass GstDemuxBinClass;

GType gst_demux_bin_get_type(void);

#endif /* __GST_DEMUXBIN_H__ */
