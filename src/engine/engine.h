#ifndef _VA_ENGINE_ENGINE_H_
#define _VA_ENGINE_ENGINE_H_

#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <cuda_runtime_api.h>

#include <iostream>
#include <stdexcept>
#include "gstnvdsmeta.h"
#include "nvds_yml_parser.h"
#include "gst-nvmessage.h"

#include "database.h"

#define MAX_DISPLAY_LEN 64

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

/* By default, OSD process-mode is set to CPU_MODE. To change mode, set as:
 * 1: GPU mode (for Tesla only)
 * 2: HW mode (For Jetson only)
 */
#define OSD_PROCESS_MODE 1

/* By default, OSD will not display text. To display text, change this to 1 */
#define OSD_DISPLAY_TEXT 1

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 40000

#define TILED_OUTPUT_WIDTH 1280
#define TILED_OUTPUT_HEIGHT 720

/* NVIDIA Decoder source pad memory feature. This feature signifies that source
 * pads having this capability will push GstBuffers containing cuda buffers. */
#define GST_CAPS_FEATURES_NVMM "memory:NVMM"

struct VAEngine {
	GMainLoop* m_loop;
	GstElement* m_pipeline;
	GstElement* m_streammux;
	GstElement* m_sink;
	GstElement* m_pgie;
	GstElement* m_queue1;
	GstElement* m_queue2;
	GstElement* m_queue3;
	GstElement* m_queue4;
	GstElement* m_queue5;
	GstElement* m_nvvidconv;
	GstElement* m_nvosd;
	GstElement* m_tiler;
	GstElement* m_nvdslogger;
	GstElement* m_transform;
	GstBus* m_bus;
	guint m_bus_watch_id;
	GstPad* m_tiler_src_pad;
	guint m_i = 0, m_num_sources = 0;
	guint m_tiler_rows, m_tiler_columns;
	guint m_pgie_batch_size;
	struct cudaDeviceProp m_cuda_prop;
	VADatabase* m_va_database;

	int m_argc;
	char** m_argv;

	VAEngine(int argc, char** argv);
	~VAEngine();

	void init();
	inline void m_calculate_num_sources(GList** src_list);
	GstElement* m_create_source_bin(guint index, gchar* uri);

	void set_database(VADatabase* _va_database);
};

#endif
