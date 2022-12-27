#include "va_object_meta.h"

VAObjectMetadata::VAObjectMetadata(NvDsObjectMeta* _metadata, std::string _object_label, guint64 _timestamp) :
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

VAObjectMetadata::~VAObjectMetadata() { }

std::ostream& operator<<(std::ostream& os, const VAObjectMetadata& bbox) {
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

VAFrameMetadata::VAFrameMetadata(guint64 _timestamp) :
	timestamp(_timestamp)
{ }

VAFrameMetadata::~VAFrameMetadata() {

}

void VAFrameMetadata::add_bbox(VAObjectMetadata* bbox) {
	va_object_meta_list.push_back(*bbox);
}
