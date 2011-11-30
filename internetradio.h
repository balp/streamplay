/**
 * Handle a internet radio stream using GStreamer
 *
 * The obejct will connect and start playing a
 * internet radio stream.
 */
class InternetRadio {
    public:
        InternetRadio(std::string uri, GMainLoop* loop);
        ~InternetRadio();
        int getBufferFill() const {
            return m_bufferFill;
        }
        const std::string getOrganization() const;
        const std::string getGenre() const;
        const std::string getLocation() const;
        const std::string getTitle() const;
    private:
	GMainLoop* m_loop;
	GstState m_state;
        GstElement* m_playbin;
        std::string m_title;
        std::string m_orgainzation;
        std::string m_genre;
        std::string m_location;
        int m_bufferFill;

        static gboolean bussCallBack(GstBus* bus, GstMessage* msg,
                gpointer instance);
        void handle_tags(GstTagList* tags);

        void setState(GstState state) { m_state = state; }
        void setOrganization(std::string organization) {
            m_orgainzation = organization;
        }
        void setGenre(std::string genre) { m_genre = genre; }
        void setLocation(std::string location) { m_location = location; }
        void setTitle(std::string title) { m_title = title; }
        void setBufferFill(int fill) { m_bufferFill = fill; }
};
