#ifndef _VA_ENGINE_VA_OBJECT_META_H_
#define _VA_ENGINE_VA_OBJECT_META_H_

#include <string>
#include <ostream>
#include <vector>

#include <glib.h>
#include "gstnvdsmeta.h"

/**
 * Create bounding box from NvDs_RectParams
 */
struct VAObjectMetadata {
	std::string object_label;
	int class_id;
	double left;
	double top;
	double width;
	double height;
	uint64_t timestamp;

	VAObjectMetadata(NvDsObjectMeta* _metadata, std::string _object_label, guint64 _timestamp);
	~VAObjectMetadata();
	friend std::ostream& operator<<(std::ostream& os, const VAObjectMetadata& bbox);
};

struct VAFrameMetadata {
	uint64_t timestamp;
	std::vector<VAObjectMetadata> va_object_meta_list {};

	VAFrameMetadata(guint64 _timestamp);
	~VAFrameMetadata();
	friend std::ostream& operator<<(std::ostream& os, const VAFrameMetadata& va_frame_meta);

	void add_bbox(VAObjectMetadata* bbox);
};

#endif
