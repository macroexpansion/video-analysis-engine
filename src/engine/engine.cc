#include "engine.h"

#include <cstring>
#include <memory>

#include "va_object_meta.h"

static gchar pgie_classes[4][32] = { "Vehicle", "TwoWheeler", "Person", "RoadSign" };

static gboolean PERF_MODE = FALSE;

/**
 * Check if running using config file or .h264 stream file
 */
static bool is_using_config_file(char* argv) {
	return (g_str_has_suffix(argv, ".yml") || g_str_has_suffix(argv, ".yaml")) ? true : false;
}

/** 
 * Extract metadata, NvDsBatchMeta -> NvDsFrameMeta -> (....) 
 */
static GstPadProbeReturn tiler_src_pad_buffer_probe(GstPad* pad, GstPadProbeInfo* info, gpointer u_data) {
	guint num_rects = 0; 
	guint vehicle_count = 0;
	guint person_count = 0;
	NvDsMetaList* l_frame = nullptr;
	NvDsMetaList* l_obj = nullptr;
	NvDsObjectMeta* object_meta = nullptr;
	// NvDsDisplayMeta* display_meta = nullptr;
	NvDsFrameMeta* frame_meta = nullptr;

	GstBuffer* buf = (GstBuffer*)(info->data);
	NvDsBatchMeta* batch_meta = gst_buffer_get_nvds_batch_meta(buf);

	for (l_frame = batch_meta->frame_meta_list; l_frame != nullptr; l_frame = l_frame->next) {
		frame_meta = (NvDsFrameMeta*)(l_frame->data);
		
		VAFrameMetadata va_frame_meta {
			frame_meta->ntp_timestamp // frame timestamp
		};
		for (l_obj = frame_meta->obj_meta_list; l_obj != nullptr; l_obj = l_obj->next) {
			object_meta = (NvDsObjectMeta*)(l_obj->data);

			VAObjectMetadata bbox {
				object_meta, // metadata
				pgie_classes[object_meta->class_id], // label
				frame_meta->ntp_timestamp // frame timestamp
			};
			va_frame_meta.add_bbox(&bbox);

			if (object_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
				vehicle_count++;
				num_rects++;
			}
			if (object_meta->class_id == PGIE_CLASS_ID_PERSON) {
				person_count++;
				num_rects++;
			}
		}
		// std::cout << va_frame_meta.va_object_meta_list.size() << std::endl;

		/* g_print(
			"Frame Number = %d Number of objects = %d " "Vehicle Count = %d Person Count = %d\n",
			frame_meta->frame_num,
			num_rects,
			vehicle_count,
			person_count
		); */

#if 0
		int offset = 0;
		display_meta = nvds_acquire_display_meta_from_pool(batch_meta);

		NvOSD_TextParams* txt_params = &display_meta->text_params[0];
		txt_params->display_text = (char*)g_malloc0(MAX_DISPLAY_LEN); // g_malloc0 return void*, cast from void* to char*
		offset = snprintf(txt_params->display_text, MAX_DISPLAY_LEN, "Person = %d ", person_count);
		offset = snprintf(txt_params->display_text + offset , MAX_DISPLAY_LEN, "Vehicle = %d ", vehicle_count);
		
		/* Now set the offsets where the string should appear */
		txt_params->x_offset = 10;
		txt_params->y_offset = 12;

		/* Font , font-color and font-size */
		txt_params->font_params.font_name = "Serif";
		txt_params->font_params.font_size = 10;
		txt_params->font_params.font_color.red = 1.0;
		txt_params->font_params.font_color.green = 1.0;
		txt_params->font_params.font_color.blue = 1.0;
		txt_params->font_params.font_color.alpha = 1.0;

		/* Text background color */
		txt_params->set_bg_clr = 1;
		txt_params->text_bg_clr.red = 0.0;
		txt_params->text_bg_clr.green = 0.0;
		txt_params->text_bg_clr.blue = 0.0;
		txt_params->text_bg_clr.alpha = 1.0;

		nvds_add_display_meta_to_frame(frame_meta, display_meta);
#endif
	}

	return GST_PAD_PROBE_OK;
}

static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
	GMainLoop* loop = (GMainLoop*)data;
	switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			g_print("End of stream\n");
			g_main_loop_quit(loop);
			break;
		case GST_MESSAGE_WARNING:
			{
				gchar* debug;
				GError* error;
				gst_message_parse_warning(msg, &error, &debug);
				g_printerr("WARNING from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
				g_free(debug);
				g_printerr("Warning: %s\n", error->message);
				g_error_free(error);
				break;
			}
		case GST_MESSAGE_ERROR:
			{
				gchar* debug;
				GError* error;
				gst_message_parse_error(msg, &error, &debug);
				g_printerr("ERROR from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
				if (debug) {
					g_printerr("Error details: %s\n", debug);
				}
				g_free(debug);
				g_error_free(error);
				g_main_loop_quit(loop);
				break;
			}
		case GST_MESSAGE_ELEMENT:
			{
				if (gst_nvmessage_is_stream_eos(msg)) {
					guint stream_id;
					if (gst_nvmessage_parse_stream_eos(msg, &stream_id)) {
						g_print("Got EOS from stream %d\n", stream_id);
					}
				}
				break;
			}
		default:
			break;
	}
	return TRUE;
}

static void cb_pad_added(GstElement* decodebin, GstPad* decoder_src_pad, gpointer data) {
	GstCaps* caps = gst_pad_get_current_caps(decoder_src_pad);
	if (!caps) {
		caps = gst_pad_query_caps(decoder_src_pad, NULL);
	}
	const GstStructure *str = gst_caps_get_structure(caps, 0);
	const gchar *name = gst_structure_get_name(str);
	GstElement *source_bin = (GstElement*)data;
	GstCapsFeatures *features = gst_caps_get_features(caps, 0);

	/* Need to check if the pad created by the decodebin is for video and not
	 * audio. */
	if (!strncmp(name, "video", 5)) {
		/* Link the decodebin pad only if decodebin has picked nvidia
		 * decoder plugin nvdec_*. We do this by checking if the pad caps contain
		 * NVMM memory features. */
		if (gst_caps_features_contains(features, GST_CAPS_FEATURES_NVMM)) {
			/* Get the source bin ghost pad */
			GstPad *bin_ghost_pad = gst_element_get_static_pad(source_bin, "src");
			if (!gst_ghost_pad_set_target(GST_GHOST_PAD(bin_ghost_pad), decoder_src_pad)) {
				g_printerr("Failed to link decoder src pad to source bin ghost pad\n");
			}
			gst_object_unref(bin_ghost_pad);
		} else {
			g_printerr("Error: Decodebin did not pick nvidia decoder plugin.\n");
		}
	}
}

static void cb_decodebin_child_added(GstChildProxy* child_proxy, GObject* object, gchar* name, gpointer user_data) {
	g_print("Decodebin child added: %s\n", name);
	if (g_strrstr(name, "decodebin") == name) {
		g_signal_connect(G_OBJECT(object), "child-added", G_CALLBACK(cb_decodebin_child_added), user_data);
	}
	if (g_strrstr(name, "source") == name) {
		g_object_set(G_OBJECT(object), "drop-on-latency", true, NULL);
	}
}

void VAEngine::init() {
	/* Standard GStreamer initialization */
	gst_init(&m_argc, &m_argv);
	m_loop = g_main_loop_new(NULL, FALSE);

	/* Create gstreamer elements */
	/* Create Pipeline element that will form a connection of other elements */
	m_pipeline = gst_pipeline_new("pipeline");
	if (!m_pipeline) {
		throw std::runtime_error("Pipeline element could not be created. Exiting.\n");
	}
	g_print("Pipeline created\n");

	/* Create nvstreammux instance to form batches from one or more sources. */
	m_streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
	if (!m_streammux) {
		throw std::runtime_error("Streamux element could not be created. Exiting.\n");
	}
	g_print("Streamux created\n");
	gst_bin_add(GST_BIN(m_pipeline), m_streammux);

	GList* src_list = nullptr;
	m_calculate_num_sources(&src_list);

	for (guint i = 0; i < m_num_sources; i++) {
		GstPad *sinkpad, *srcpad;
		gchar pad_name[16] = { };

		GstElement* source_bin= nullptr;
		if (is_using_config_file(m_argv[1])) {
			g_print("Now playing : %s\n", (char*)(src_list)->data);
			source_bin = m_create_source_bin(i, (char*)(src_list)->data);
		} else {
			source_bin = m_create_source_bin(i, m_argv[i + 1]);
		}
		if (!source_bin) {
			throw std::runtime_error("Failed to create source bin. Exiting.\n");
		}

		gst_bin_add(GST_BIN(m_pipeline), source_bin);

		g_snprintf(pad_name, 15, "sink_%u", i);
		sinkpad = gst_element_get_request_pad(m_streammux, pad_name);
		if (!sinkpad) {
			throw std::runtime_error("Streammux request sink pad failed. Exiting.\n");
		}

		srcpad = gst_element_get_static_pad(source_bin, "src");
		if (!srcpad) {
			throw std::runtime_error("Failed to get src pad of source bin. Exiting.\n");
		}

		if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
			throw std::runtime_error("Failed to link source bin to stream muxer. Exiting.\n");
		}

		gst_object_unref(srcpad);
		gst_object_unref(sinkpad);

		if (is_using_config_file(m_argv[1])) {
			src_list = src_list->next;
		}
	}

	if (is_using_config_file(m_argv[1])) {
		g_list_free(src_list);
	}

	/* Use nvinfer to infer on batched frame. */
	m_pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");
	if (!m_pgie) {
		throw std::runtime_error("Pgie element could not be created. Exiting.\n");
	}
	g_print("Pgie created\n");

	/* Add queue elements between every two elements */
	m_queue1 = gst_element_factory_make("queue", "queue1");
	m_queue2 = gst_element_factory_make("queue", "queue2");
	m_queue3 = gst_element_factory_make("queue", "queue3");
	m_queue4 = gst_element_factory_make("queue", "queue4");
	m_queue5 = gst_element_factory_make("queue", "queue5");

	/* Use nvdslogger for perf measurement. */
	m_nvdslogger = gst_element_factory_make ("nvdslogger", "nvdslogger");
	if (!m_nvdslogger) {
		throw std::runtime_error("NvDsLogger element could not be created. Exiting.\n");
	}
	g_print("NvDsLogger created\n");

	/* Use nvtiler to composite the batched frames into a 2D tiled array based
	 * on the source of the frames. */
	m_tiler = gst_element_factory_make("nvmultistreamtiler", "nvtiler");
	if (!m_tiler) {
		throw std::runtime_error("Tiler element could not be created. Exiting.\n");
	}
	g_print("Tiler created\n");

	/* Use convertor to convert from NV12 to RGBA as required by nvosd */
	m_nvvidconv = gst_element_factory_make("nvvideoconvert", "nvvideo-converter");
	if (!m_nvvidconv) {
		throw std::runtime_error("NvVidConv element could not be created. Exiting.\n");
	}
	g_print("NvVidConv created\n");

	/* Create OSD to draw on the converted RGBA buffer */
	m_nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
	if (!m_nvosd) {
		throw std::runtime_error("NvOsd element could not be created. Exiting.\n");
	}
	g_print("NvOsd created\n");

	if (PERF_MODE) {
		m_sink = gst_element_factory_make("fakesink", "nvvideo-renderer");
	} else {
		/* Finally render the osd output */
		if (m_cuda_prop.integrated) {
			m_transform = gst_element_factory_make("nvegltransform", "nvegl-transform");
			if (!m_transform) {
				throw std::runtime_error("One Tegra element could not be created. Exiting.\n");
			}
			g_print("NvEglTransform created\n");
		}
		m_sink = gst_element_factory_make("nveglglessink", "nvvideo-renderer");
	}
	if (!m_sink) {
		throw std::runtime_error("Sink element could not be created. Exiting.\n");
	}
	g_print("Sink created\n");

	m_tiler_rows = (guint)sqrt(m_num_sources);
	m_tiler_columns = (guint)ceil(1.0 * m_num_sources / m_tiler_rows);
	if (is_using_config_file(m_argv[1])) {
		nvds_parse_streammux(m_streammux, m_argv[1], "streammux");
		g_object_set(G_OBJECT(m_pgie), "config-file-path", "configs/pgie_config.yml", NULL);
		g_object_get(G_OBJECT(m_pgie), "batch-size", &m_pgie_batch_size, NULL);
		if (m_pgie_batch_size != m_num_sources) {
			g_printerr("WARNING: Overriding infer-config batch-size (%d) with number of sources (%d)\n", m_pgie_batch_size, m_num_sources);
			g_object_set(G_OBJECT(m_pgie), "batch-size", m_num_sources, NULL);
		}

		nvds_parse_osd(m_nvosd, m_argv[1], "osd");
		g_object_set(G_OBJECT(m_tiler), "rows", m_tiler_rows, "columns", m_tiler_columns, NULL);

		nvds_parse_tiler(m_tiler, m_argv[1], "tiler");
		nvds_parse_egl_sink(m_sink, m_argv[1], "sink");
	} else {
		g_object_set(G_OBJECT(m_streammux), "batch-size", m_num_sources, NULL);
		g_object_set(
			G_OBJECT(m_streammux),
			"width",
			MUXER_OUTPUT_WIDTH,
			"height",
			MUXER_OUTPUT_HEIGHT,
			"batched-push-timeout",
			MUXER_BATCH_TIMEOUT_USEC,
			NULL
		);

		/* Configure the nvinfer element using the nvinfer config file. */
		g_object_set(G_OBJECT(m_pgie), "config-file-path", "configs/pgie_config.txt", NULL);

		/* Override the batch-size set in the config file with the number of sources. */
		g_object_get (G_OBJECT(m_pgie), "batch-size", &m_pgie_batch_size, NULL);
		if (m_pgie_batch_size != m_num_sources) {
			g_printerr("WARNING: Overriding infer-config batch-size (%d) with number of sources (%d)\n", m_pgie_batch_size, m_num_sources);
			g_object_set(G_OBJECT(m_pgie), "batch-size", m_num_sources, NULL);
		}

		/* we set the tiler properties here */
		g_object_set(
			G_OBJECT(m_tiler),
			"rows",
			m_tiler_rows,
			"columns",
			m_tiler_columns,
			"width",
			TILED_OUTPUT_WIDTH,
			"height",
			TILED_OUTPUT_HEIGHT,
			NULL
		);

		g_object_set(G_OBJECT(m_nvosd), "process-mode", OSD_PROCESS_MODE, "display-text", OSD_DISPLAY_TEXT, NULL);
		g_object_set(G_OBJECT(m_sink), "qos", 0, NULL);
	}

	/* we add a message handler */
	m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	m_bus_watch_id = gst_bus_add_watch(m_bus, bus_call, m_loop);
	gst_object_unref(m_bus);

	/* Set up the pipeline */
	/* we add all elements into the pipeline */
	if (m_transform) {
		gst_bin_add_many(GST_BIN(m_pipeline), m_queue1, m_pgie, m_queue2, m_nvdslogger, m_tiler,
				m_queue3, m_nvvidconv, m_queue4, m_nvosd, m_queue5, m_transform, m_sink, NULL);
		/* we link the elements together
		 * nvstreammux -> nvinfer -> nvdslogger -> nvtiler -> nvvidconv -> nvosd
		 * -> video-renderer */
		if (!gst_element_link_many(m_streammux, m_queue1, m_pgie, m_queue2, m_nvdslogger, m_tiler,
					m_queue3, m_nvvidconv, m_queue4, m_nvosd, m_queue5, m_transform, m_sink, NULL)) {
			throw std::runtime_error("Elements could not be linked. Exiting.\n");
		}
	} else {
		gst_bin_add_many(GST_BIN(m_pipeline), m_queue1, m_pgie, m_queue2, m_nvdslogger, m_tiler,
				m_queue3, m_nvvidconv, m_queue4, m_nvosd, m_queue5, m_sink, NULL);
		// gst_bin_add_many(GST_BIN(m_pipeline), m_pgie, m_nvdslogger, m_tiler, m_nvvidconv, m_nvosd, m_sink, NULL);
		/* we link the elements together
		 * nvstreammux -> nvinfer -> nvdslogger -> nvtiler -> nvvidconv -> nvosd
		 * -> video-renderer */
		if (!gst_element_link_many(m_streammux, m_queue1, m_pgie, m_queue2, m_nvdslogger, m_tiler,
					m_queue3, m_nvvidconv, m_queue4, m_nvosd, m_queue5, m_sink, NULL)) {
			throw std::runtime_error("Elements could not be linked. Exiting.\n");
		}
		/* if (!gst_element_link_many(m_streammux, m_pgie, m_nvdslogger, m_tiler, m_nvvidconv, m_nvosd, m_sink, NULL)) {
			throw std::runtime_error("Elements could not be linked. Exiting.\n");
		} */
	}

	/* Lets add probe to get informed of the meta data generated, we add probe to
	 * the sink pad of the osd element, since by that time, the buffer would have
	 * had got all the metadata. */
	m_tiler_src_pad = gst_element_get_static_pad(m_pgie, "src");
	if (!m_tiler_src_pad) {
		g_print("Unable to get src pad\n");
	} else {
		gst_pad_add_probe(m_tiler_src_pad, GST_PAD_PROBE_TYPE_BUFFER, tiler_src_pad_buffer_probe, NULL, NULL);
	}
	gst_object_unref(m_tiler_src_pad);

	/* Set the pipeline to "playing" state */
	if (is_using_config_file(m_argv[1])) {
		g_print("Using file: %s\n", m_argv[1]);
	} else {
		g_print("Now playing:");
		for (guint i = 0; i < m_num_sources; i++) {
			g_print(" %s,", m_argv[i + 1]);
		}
		g_print("\n");
	}
	PERF_MODE = g_getenv("PERF_MODE") && !g_strcmp0(g_getenv("PERF_MODE"), "1");
	gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

	/* Wait till pipeline encounters an error or EOS */
	g_print("Running...\n");
	g_main_loop_run(m_loop);

	/* Out of the main loop, clean up nicely */
	g_print("Returned, stopping playback\n");
	gst_element_set_state(m_pipeline, GST_STATE_NULL);
	g_print("Deleting pipeline\n");
	gst_object_unref(GST_OBJECT(m_pipeline));
	g_source_remove(m_bus_watch_id);
	g_main_loop_unref(m_loop);
}

void VAEngine::m_calculate_num_sources(GList** src_list) {
	if (is_using_config_file(m_argv[1])) {
		nvds_parse_source_list(src_list, m_argv[1], "source-list");
		GList* temp_src_list = *src_list;
		while (temp_src_list) {
			m_num_sources++;
			temp_src_list = temp_src_list->next;
		}
		g_list_free(temp_src_list);
	} else {
		m_num_sources = m_argc - 1;
	}
}

GstElement* VAEngine::m_create_source_bin(guint index, gchar* uri) {
	GstElement *bin = nullptr, *uri_decode_bin = nullptr;
	gchar bin_name[16] = { };

	g_snprintf(bin_name, 15, "source-bin-%02d", index);
	/* Create a source GstBin to abstract this bin's content from the rest of the
	 * pipeline */
	bin = gst_bin_new(bin_name);

	/* Source element for reading from the uri.
	 * We will use decodebin and let it figure out the container format of the
	 * stream and the codec and plug the appropriate demux and decode plugins. */
	if (PERF_MODE) {
		uri_decode_bin = gst_element_factory_make("nvurisrcbin", "uri-decode-bin");
		g_object_set(G_OBJECT(uri_decode_bin), "file-loop", TRUE, NULL);
	} else {
		uri_decode_bin = gst_element_factory_make("uridecodebin", "uri-decode-bin");
	}

	if (!bin || !uri_decode_bin) {
		throw std::runtime_error("One element in source bin could not be created.\n");
	}

	/* We set the input uri to the source element */
	g_object_set(G_OBJECT(uri_decode_bin), "uri", uri, NULL);

	/* Connect to the "pad-added" signal of the decodebin which generates a
	 * callback once a new pad for raw data has beed created by the decodebin */
	g_signal_connect(G_OBJECT(uri_decode_bin), "pad-added", G_CALLBACK(cb_pad_added), bin);
	g_signal_connect(G_OBJECT(uri_decode_bin), "child-added", G_CALLBACK(cb_decodebin_child_added), bin);
	gst_bin_add(GST_BIN(bin), uri_decode_bin);

	/* We need to create a ghost pad for the source bin which will act as a proxy
	 * for the video decoder src pad. The ghost pad will not have a target right
	 * now. Once the decode bin creates the video decoder and generates the
	 * cb_pad_added callback, we will set the ghost pad target to the video decoder
	 * src pad. */
	if (!gst_element_add_pad(bin, gst_ghost_pad_new_no_target("src", GST_PAD_SRC))) {
		throw std::runtime_error("Failed to add ghost pad in source bin\n");
	}

	return bin;
}

void VAEngine::set_database(VADatabase* _va_database) {
	m_va_database = _va_database;
}

VAEngine::VAEngine(int argc, char** argv) : m_argc(argc), m_argv(argv) {
	/* Check input arguments */
	if (m_argc < 2) {
		g_printerr("Usage: %s <yml file>\n", m_argv[0]);
		g_printerr("OR: %s <uri1> [uri2] ... [uriN] \n", m_argv[0]);
		throw std::invalid_argument("invalid argument\n");
	}

	int current_device = -1; // automatically find CUDA device
	cudaGetDevice(&current_device);
	cudaGetDeviceProperties(&m_cuda_prop, current_device);
	g_print("current CUDA device: %d\n", current_device);
}

VAEngine::~VAEngine() {
	std::cout << "start VA ENGINE deallocate" << std::endl;
	std::cout << "finish VA ENGINE deallocate" << std::endl;
}
