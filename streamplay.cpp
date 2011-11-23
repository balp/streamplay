#include <iostream>
#include <string>
#include <gst/gst.h>



class InternetRadio {
    public:
        InternetRadio(std::string uri);
        ~InternetRadio();
        int getBufferFill() {
            return m_bufferFill;
        }
        std::string getOrganization();
        std::string getGenre();
        std::string getLocation();
        std::string getTitle();
    private:
        GstElement* m_playbin;
        std::string m_title;
        std::string m_orgainzation;
        std::string m_genre;
        std::string m_location;
        int m_bufferFill;

        static gboolean bussCallBack(GstBus* bus, GstMessage* msg,
                gpointer instance);
        void handle_tags(GstTagList* tags);

        void setOrganization(std::string organization) {
            m_orgainzation = organization;
        }
        void setGenre(std::string genre) { m_genre = genre; }
        void setLocation(std::string location) { m_location = location; }
        void setTitle(std::string title) { m_title = title; }
        void setBufferFill(int fill) { m_bufferFill = fill; }
};

InternetRadio::InternetRadio(std::string uri) : m_playbin(NULL),
    m_title("[unkown]"), m_orgainzation("[unkown]"), m_bufferFill(0)
{
    // Setup stream
    //std::cout << "Opening source " << uri << std::endl;
    m_playbin = gst_element_factory_make("playbin", "play");
    g_object_set( G_OBJECT(m_playbin), "uri", uri.c_str(), NULL );

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
    gst_bus_add_watch(bus, bussCallBack, this);
    gst_object_unref(bus);

    //std::cout << "Now playing: " << uri << std::endl;
    gst_element_set_state(m_playbin, GST_STATE_PLAYING);
}

InternetRadio::~InternetRadio()
{
    gst_element_set_state(m_playbin, GST_STATE_NULL);
    gst_object_unref(m_playbin);
}

static GMainLoop* loop;

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
#if 0
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
    gst_tag_list_foreach(tags, print_one_tag, NULL);
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
                    g_main_loop_quit ( loop );
                    break;
                }
            case GST_MESSAGE_EOS:
                g_main_loop_quit ( loop );
                break;
            case GST_MESSAGE_STREAM_STATUS:
                {
                    GstStreamStatusType type;
                    GstElement* owner;
                    gst_message_parse_stream_status(msg, &type, &owner);
                    std::cout << "Stream status now " << type << std::endl;
                }
                break;
            case GST_MESSAGE_STATE_CHANGED:
                {
                    GstState oldState;
                    GstState newState;
                    gst_message_parse_state_changed(msg, &oldState, &newState, NULL);
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
            case GST_MESSAGE_BUFFERING:
                {
                    gint percent;
                    gst_message_parse_buffering(msg, &percent);
                    radio->m_bufferFill = percent;

                    std::cout << "Buffering: " << percent << "%" << std::endl;
                }
                break;
            case GST_MESSAGE_DURATION:
                {
                    GstFormat format;
                    gint64 duration;
                    gst_message_parse_duration(msg, &format, &duration);
                    std::cout << "Duration: " << format << "-" << duration << std::endl;

                }
                break;
            case GST_MESSAGE_TAG:
                {
                    GstTagList* tags = NULL;
                    gst_message_parse_tag(msg, &tags);
                    std::cout << "Tags: " << GST_OBJECT_NAME(msg->src) << std::endl;
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

#if 0
static void cb_typefound(GstElement* , guint probability,
        GstCaps* caps, gpointer )
{
    //GMainLoop* loop = data;

    gchar* type = gst_caps_to_string(caps);
    std::cout << "Stream of " << type << " found, propability " << probability << "%" << std::endl;
    g_free(type);
}
#endif

int main(int argc, char** argv)
{
    // Options parser
    GOptionContext* ctx = g_option_context_new("streamplay");
    const gchar** remaining_args = NULL;
    GOptionEntry entries[] = {
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY,
            &remaining_args, NULL, "FILE" },
        { NULL, '\0', 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
    };
    g_option_context_add_main_entries(ctx, entries, NULL);
    g_option_context_add_group(ctx, gst_init_get_option_group() );

    GError* err = NULL;
    if(!g_option_context_parse(ctx, &argc, &argv, &err)) {
        std::cerr << "Argument error:" << err->message << std::endl;
        g_error_free(err);
        return 1;
    }
    if(! remaining_args) { // Missing url argument
        std::cerr << "Usage: [uri]" << std::endl;
        return 1;
    }

    const gchar* uri = remaining_args[0];

    // GStreamer version info
    guint major, minor, micro, nano;
    gst_init(&argc, &argv);
    gst_version(&major, &minor, &micro, &nano);
    const gchar *nano_str = "";
    switch(nano) {
        case 1:
            nano_str = "(CVS)";
            break;
        case 2:
            nano_str = "(Prerelease)";
            break;
    } 
    std::cout << "Using GStreamer " << major << "." << minor << "." << micro << " " << nano_str << std::endl;

    // Start main loop
    {
        InternetRadio radio(uri);
        loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(loop);
    }

    g_main_loop_unref(loop);

    return 0;
}
