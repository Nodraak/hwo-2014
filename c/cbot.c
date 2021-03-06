
/* fix ci compile error about struct addrinfo */
#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include "cJSON.h"
#include "cbot.h"
#include "ft_utils.h"


void error(char *fmt, ...)
{
    char buf[BUFSIZ];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    if (errno)
        perror(buf);
    else
        fprintf(stderr, "%s\n", buf);

    exit(1);
}

int connect_to(char *hostname, char *port)
{
    int status;
    int fd;
    char portstr[32];
    struct addrinfo hint;
    struct addrinfo *info;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = PF_INET;
    hint.ai_socktype = SOCK_STREAM;

    sprintf(portstr, "%d", atoi(port));

    status = getaddrinfo(hostname, portstr, &hint, &info);
    if (status != 0) error("failed to get address: %s", gai_strerror(status));

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) error("failed to create socket");

    status = connect(fd, info->ai_addr, info->ai_addrlen);
    if (status < 0) error("failed to connect to server");

    freeaddrinfo(info);
    return fd;
}

void log_message(char *msg_type_name, cJSON *msg)
{
    cJSON *msg_data;

    if (!strcmp("join", msg_type_name)) {
        puts("Joined");
    } else if (!strcmp("gameStart", msg_type_name)) {
        puts("Race started");
    } else if (!strcmp("crash", msg_type_name)) {
        puts("Someone crashed");
    } else if (!strcmp("gameEnd", msg_type_name)) {
        puts("Race ended");
    } else if (!strcmp("error", msg_type_name)) {
        msg_data = cJSON_GetObjectItem(msg, "data");
        if (msg_data == NULL)
            puts("Unknown error");
        else
            printf("ERROR: %s\n", msg_data->valuestring);
    }
}

int main(int argc, char *argv[])
{
    int sock, notAutoBuild;
    cJSON *json;

    if (argc != 5)
        error("Usage: bot host port botname botkey\n");

#ifdef NOT_AUTO_BUILD
    notAutoBuild = 1;
#else
    notAutoBuild = 0;
#endif
    printf("join info : NOT_AUTO_BUILD=%d nbPlayer=%d trackName(if notAutoBuild)=%s\n",
        notAutoBuild, NB_PLAYER, TRACK_NAME);

    sock = connect_to(argv[1], argv[2]);

    json = join_msg(argv[3], argv[4]);
    write_msg(sock, json);
    cJSON_Delete(json);

    ft_main_loop(sock, argv[3]);

    return 0;
}

cJSON *ping_msg()
{
    return make_msg("ping", cJSON_CreateString("ping"));
}

cJSON *join_msg(char *bot_name, char *bot_key)
{
    cJSON *botId = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

#ifdef NOT_AUTO_BUILD
    #ifndef TRACK_NAME
        #error a track name must be defined
    #endif

    cJSON_AddStringToObject(botId, "name", bot_name);
    cJSON_AddStringToObject(botId, "key", bot_key);
    cJSON_AddItemToObject(data, "botId", botId);
    cJSON_AddNumberToObject(data, "carCount", NB_PLAYER);
    cJSON_AddStringToObject(data, "trackName", TRACK_NAME);

    return make_msg("joinRace", data);
#else
    (void)botId; /* disable warning */

    cJSON_AddStringToObject(data, "name", bot_name);
    cJSON_AddStringToObject(data, "key", bot_key);

    return make_msg("join", data);
#endif
}

cJSON *throttle_msg(double throttle)
{
    return make_msg("throttle", cJSON_CreateNumber(throttle));
}

cJSON *make_msg(char *type, cJSON *data)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "msgType", type);
    cJSON_AddItemToObject(json, "data", data);
    return json;
}

cJSON *read_msg(int fd)
{
    int bufsz, readsz;
    char *readp, *buf;
    cJSON *json = NULL;

    bufsz = 16;
    readsz = 0;
    readp = buf = malloc(bufsz * sizeof(char));

    while (read(fd, readp, 1) > 0) {
        if (*readp == '\n')
            break;

        readp++;
        if (++readsz == bufsz) {
            buf = realloc(buf, bufsz *= 2);
            readp = buf + readsz;
        }
    }

    if (readsz > 0) {
        *readp = '\0';
        json = cJSON_Parse(buf);
        if (json == NULL)
            error("malformed JSON(%s): %s", cJSON_GetErrorPtr(), buf);
    }
    free(buf);
    return json;
}

void write_msg(int fd, cJSON *msg)
{
    char nl = '\n';
    char *msg_str;

    msg_str = cJSON_PrintUnformatted(msg);

    write(fd, msg_str, strlen(msg_str));
    write(fd, &nl, 1);

    free(msg_str);
}
