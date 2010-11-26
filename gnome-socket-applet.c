#include <string.h>
#include <stdlib.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <resolv.h>

/* Pipe state */
typedef struct {
    PanelApplet *applet;
    GtkLabel *label;
} socket_data;

/* Thread running, reading from the socket */
void *socket_thread(void *pthread_data) {
    socket_data *data = (socket_data *) pthread_data;
    int buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    int sock_fd;
    struct sockaddr_in self;

    /* Create the socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    /* Initialize address/port structure */
    bzero(&self, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(atoi(getenv("GNOME_SOCKET_APPLET_PORT")));
    self.sin_addr.s_addr = INADDR_ANY;

    /* Bind and listen */
    bind(sock_fd, (struct sockaddr *) &self, sizeof(self));
    listen(sock_fd, 20);

    /* Forever... */
    while(1) {
        int client_fd;
        struct sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);

        /* Accept the socket */
        client_fd = accept(sock_fd,
                (struct sockaddr *) &client_addr, &client_addr_size);

        /* Check that the message comes from localhost */
        inet_ntop(AF_INET, (void *) &client_addr.sin_addr, buffer, buffer_size);
        if(!strcmp(buffer, getenv("GNOME_SOCKET_APPLET_HOST"))) {
            /* Receive and set text */
            bzero(buffer, buffer_size);
            recv(client_fd, buffer, buffer_size - 1, 0);
            gtk_label_set_text(data->label, buffer);
        }

        close(client_fd);
    }

    close(sock_fd);
    free(buffer);
}

static gboolean socket_applet_fill(PanelApplet *applet, const gchar *iid,
        gpointer g_data) {
    GtkWidget *label;
    pthread_t thread;
    socket_data *data = malloc(sizeof(socket_data));

    if (strcmp (iid, "OAFIID:GnomeSocketApplet") != 0)
        return FALSE;

    label = gtk_label_new("Waiting for connection...");
    gtk_container_add(GTK_CONTAINER (applet), label);
    gtk_widget_show_all(GTK_WIDGET (applet));

    /* Start the socket */
    data->applet = applet;
    data->label = (GtkLabel *) label;
    pthread_create(&thread, NULL, socket_thread, data);

    return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY(
        "OAFIID:GnomeSocketApplet_Factory", PANEL_TYPE_APPLET,
        "Gnome Socket Applet", "0", socket_applet_fill, NULL);
