#include "mongoose.h"


struct device_settings {
    char string[32];
};

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static struct device_settings s_settings = {""};


static void readHttpParams(struct http_message *hm) {
    char buffer[100];
    /* Get form variables and store settings values */
    mg_get_http_var(&hm->body, "message", buffer,
                    sizeof(s_settings.string));
    memcpy(s_settings.string, buffer, sizeof(s_settings.string));
}


static void handle_save(struct mg_connection *nc, struct http_message *hm) {
    readHttpParams(hm);
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n\r\n%.*s",
              (unsigned long) hm->body.len, (int) hm->body.len, hm->body.p);
}

static void handle_ssi_call(struct mg_connection *nc, const char *param) {
    if (strcmp(param, "Response") == 0) {
        mg_printf_html_escape(nc, "%s", s_settings.string);
    }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    auto *hm = (struct http_message *) ev_data;

    switch (ev) {
        case MG_EV_HTTP_REQUEST:
            if (mg_vcmp(&hm->uri, "/save") == 0) {
                handle_save(nc, hm); /* Handle RESTful call */
            } else {
                mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
            }
            break;
        case MG_EV_SSI_CALL:
            handle_ssi_call(nc, (char *) ev_data);
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    struct mg_mgr mgr{};
    struct mg_connection *nc;
    char *p, path[512];
    
    mg_mgr_init(&mgr, nullptr);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = "./web_root";
    s_http_server_opts.auth_domain = "example.com";
    if (argc > 0 && (p = strrchr(argv[0], '/'))) {
        snprintf(path, sizeof(path), "%.*s/web_root", (int) (p - argv[0]), argv[0]);
        s_http_server_opts.document_root = path;
	printf("%s\n",path);
    }
    printf("Starting device configurator on port %s\n", s_http_port);
    while (true) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}