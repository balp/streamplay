
#include <iostream>
#include <string>
#include <gst/gst.h>
#include <internetradio.h>

InternetRadio::InternetRadio(std::string uri, GMainLoop* loop)
    : m_loop(loop),
    m_state(GST_STATE_VOID_PENDING), 
    m_playbin(NULL),
    m_title("[unkown]"),
    m_orgainzation("[unkown]"),
    m_genre(""),
    m_location(""),
    m_bufferFill(0)
{
    // Setup stream
    m_playbin = gst_element_factory_make("playbin", "InternetRadio");
    g_object_set( G_OBJECT(m_playbin), "uri", uri.c_str(), NULL );

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
    gst_bus_add_watch(bus, bussCallBack, this);
    gst_object_unref(bus);

    gst_element_set_state(m_playbin, GST_STATE_PLAYING);
}

InternetRadio::~InternetRadio()
{
    gst_element_set_state(m_playbin, GST_STATE_NULL);
    gst_object_unref(m_playbin);
}


#if 0
static void print_one_tag(const GstTagList* list, const gchar* tag, gpointer )
{
    int num = gst_tag_list_get_tag_size(list, tag);
    std::cout << "Tag: " << tag << " " << num << std::endl;
    for(int i = 0; i < num; ++i) {
        const GValue* val = gst_tag_list_get_value_index(list, tag, i);
        if(G_VALUE_HOLDS_STRING(val)) {
            std::cout << i << ":" << tag << ":" << g_value_get_string(val) << std::endl;
        } else if(G_VALUE_HOLDS_UINT(val)) {
            std::cout << i << "u" << tag << ":" << g_value_get_uint(val) << std::endl;
        } else if(G_VALUE_HOLDS_DOUBLE(val)) {
            std::cout << i << "d" << tag << ":" << g_value_get_double(val) << std::endl;
        } else if(G_VALUE_HOLDS_BOOLEAN(val)) {
            std::cout << i << "b" << tag << ":" << g_value_get_boolean(val) << std::endl;
        } else {
            std::cout << i << "o" << tag << ":" << G_VALUE_TYPE_NAME(val) << std::endl;
        }

    }
}

static void handle_tags(GstTagList* tags)
{
}
#endif

static const gchar* tagListToString(GstTagList* tags, std::string tag)
{
    int num = gst_tag_list_get_tag_size(tags, tag.c_str());
    if(num) {
        const GValue* val = gst_tag_list_get_value_index(tags, tag.c_str(), 0);
        return g_value_get_string(val);
    }
    return "";
}

void InternetRadio::handle_tags(GstTagList* tags)
{
    //gst_tag_list_foreach(tags, print_one_tag, NULL);
    if( gst_tag_exists("title") ) {
        setTitle(tagListToString(tags, "title"));
    }
    if( gst_tag_exists("organization") ) {
        setOrganization(tagListToString(tags, "organization"));
    }
    if( gst_tag_exists("genre") ) {
        setGenre(tagListToString(tags, "genre"));
    }
    if( gst_tag_exists("location") ) {
        setLocation(tagListToString(tags, "location"));
    }
}

gboolean InternetRadio::bussCallBack(GstBus*, GstMessage* msg, gpointer object)
{
    if(object) { 
        InternetRadio* radio = reinterpret_cast<InternetRadio*>(object);
        switch(GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                {
                    gchar* debug;
                    GError* error;
                    gst_message_parse_error(msg, &error, &debug);
                    std::cerr << "Error: " << error->message << std::endl;
                    std::cerr << "Debug: " << debug << std::endl;
                    g_error_free(error);
                    g_main_loop_quit ( radio->m_loop );
                    break;
                }
            case GST_MESSAGE_EOS:
                g_main_loop_quit ( radio->m_loop );
                break;
            case GST_MESSAGE_STREAM_STATUS:
                {
#if 0
                    GstStreamStatusType type;
                    GstElement* owner;
                    gst_message_parse_stream_status(msg, &type, &owner);
                    std::cout << "Stream status now " << type << std::endl;
#endif
                }
                break;
#if 1
            case GST_MESSAGE_STATE_CHANGED:
                {
                    GstState oldState;
                    GstState newState;
                    gst_message_parse_state_changed(msg, &oldState, &newState, NULL);
		    if(GST_OBJECT(radio->m_playbin) == msg->src) {
			radio->setState(newState);
			//std::cout << "Player is now " << gst_element_state_get_name(newState) << std::endl;
		    }
#if 0
                    std::cout << "State " << GST_OBJECT_NAME(msg->src) 
                        << " changed from "
                        << gst_element_state_get_name (oldState)
                        << " to "
                        << gst_element_state_get_name (newState)
                        << "." << std::endl;
#endif
                }
                break;
#endif
            case GST_MESSAGE_BUFFERING:
                {
                    gint percent;
                    gst_message_parse_buffering(msg, &percent);
		    radio->setBufferFill(percent);
                }
                break;
            case GST_MESSAGE_DURATION:
                {
#if 0
                    GstFormat format;
                    gint64 duration;
                    gst_message_parse_duration(msg, &format, &duration);
                    std::cout << "Duration: " << format << "-" << duration << std::endl;
#endif

                }
                break;
            case GST_MESSAGE_TAG:
                {
                    GstTagList* tags = NULL;
                    gst_message_parse_tag(msg, &tags);
                    //std::cout << "Tags: " << GST_OBJECT_NAME(msg->src) << std::endl;
                    radio->handle_tags( tags );
                    gst_tag_list_free(tags);
                }
                break;
            case GST_MESSAGE_ASYNC_START:
            case GST_MESSAGE_ASYNC_DONE:
            case GST_MESSAGE_NEW_CLOCK:
                break; // Ignore for now
            default:
                std::cout << "Got Message (unhandeled): " << GST_MESSAGE_TYPE_NAME(msg) << std::endl;
                // Laters
                break;
        }
        return TRUE;
    }
    return FALSE;
}


