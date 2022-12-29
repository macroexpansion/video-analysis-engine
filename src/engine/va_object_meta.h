#ifndef VA_ENGINE_OBJECT_META_H_
#define VA_ENGINE_OBJECT_META_H_

#include <string>
#include <ostream>
#include <vector>

#include <glib.h>
#include "gstnvdsmeta.h"

namespace va {
/**
 * Bounding box from NvDs_RectParams
 */
struct ObjectMetadata {
	std::string object_label;
	int class_id;
	double left;
	double top;
	double width;
	double height;
	uint64_t timestamp;

	ObjectMetadata();
	ObjectMetadata(NvDsObjectMeta* _metadata, std::string _object_label, guint64 _timestamp);
	ObjectMetadata(NvDsObjectMeta* _metadata, std::string _object_label);
	~ObjectMetadata();
	friend auto operator<<(std::ostream& os, const ObjectMetadata& bbox) -> std::ostream&;
};

/**
 * Frame metadata which holds a list of ObjectMetadata from NvDs_RectParams
 */
struct FrameMetadata {
	std::string video_file;
	uint64_t timestamp;
	std::vector<ObjectMetadata> va_object_meta_list {};

	FrameMetadata();
	FrameMetadata(guint64 _timestamp);
	FrameMetadata(std::string _video_file, guint64 _timestamp);
	~FrameMetadata();
	friend auto operator<<(std::ostream& os, const FrameMetadata& va_frame_meta) -> std::ostream&;

	auto add_bbox(ObjectMetadata* bbox) -> void;
};

} // namespace va

#endif
