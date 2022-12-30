#include "va_object_meta.h"

#include <iostream>

va::ObjectMetadata::ObjectMetadata() : object_label("a"), class_id(0), left(0.0), top(0.0), width(0.0), height(0.0), timestamp(0) {}

va::ObjectMetadata::ObjectMetadata(NvDsObjectMeta* _metadata, std::string _object_label, guint64 _timestamp) :
	object_label(_object_label),
	timestamp(_timestamp)
{
	NvOSD_RectParams rect = _metadata->rect_params;
	class_id = _metadata->class_id;
	left = rect.left;
	top = rect.top;
	width = rect.width;
	height = rect.height;
}

va::ObjectMetadata::ObjectMetadata(NvDsObjectMeta* _metadata, std::string _object_label) : object_label(_object_label) {
	NvOSD_RectParams rect = _metadata->rect_params;
	class_id = _metadata->class_id;
	left = rect.left;
	top = rect.top;
	width = rect.width;
	height = rect.height;
}

va::ObjectMetadata::~ObjectMetadata() {
	// std::cout << "object metadata deallocated" << std::endl;
}

auto operator<<(std::ostream& os, const va::ObjectMetadata& bbox) -> std::ostream& {
	os << "{ ";
	os << "label: " << bbox.object_label << ", ";
	os << "class id: " << bbox.class_id << ", ";
	os << "left: " << bbox.left << ", ";
	os << "top: " << bbox.top << ", ";
	os << "width: " << bbox.width << ", ";
	os << "height: " << bbox.height << ", ";
	os << "timestamp: " << bbox.timestamp;
	os << " }";
	return os;
}

va::FrameMetadata::FrameMetadata(std::string _video_file, guint64 _timestamp) : video_file(_video_file), timestamp(_timestamp) {
	va_object_meta_list.reserve(48);
}

va::FrameMetadata::FrameMetadata(guint64 _timestamp) : timestamp(_timestamp) {
	va_object_meta_list.reserve(48);
}

va::FrameMetadata::~FrameMetadata() {
	// std::cout << "frame metadata deallocated" << std::endl;
}
