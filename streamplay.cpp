#include <iostream>
#include <string>
#include <gst/gst.h>

#include <internetradio.h>

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

static GMainLoop* loop;

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
        InternetRadio radio(uri, loop);
        loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(loop);
    }

    g_main_loop_unref(loop);

    return 0;
}
