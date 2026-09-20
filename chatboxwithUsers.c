#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

struct message
{
    long msgType;
    char msgText[1000];
};

static int is_burn_cmd(const char *s, int n)
{
    return (n > 5 &&
            s[0] == 'B' && s[1] == 'U' && s[2] == 'R' && s[3] == 'N' && s[4] == ' ');
}

static char *burn_split(char *buf, int *sec_out)
{
    char *p = buf + 5;
    while (*p == ' ' && *p)
        p++;

    char *end = p;
    long v = strtol(p, &end, 10);
    *sec_out = (int)v;

    p = end;
    while (*p == ' ' && *p)
        p++;

    return p;
}

static void print_history(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return;

    char b[1024];
    int r;
    while ((r = (int)read(fd, b, sizeof(b))) > 0)
    {
        write(1, b, r);
    }
    close(fd);
}

static void log_msg(int fd, const char *tag, const char *txt)
{
    write(fd, tag, (int)strlen(tag));
    write(fd, txt, (int)strlen(txt));
}

static void clear_screen(void)
{
    write(1, "\033[H\033[J", 6);
}

int main(void)
{
    clear_screen();

    {
        char welcomeMessage[] = "=== Welcome to Ghost Chat ===\n";
        write(1, welcomeMessage, (int)strlen(welcomeMessage));
    }

    int who = 0;
    int flag = open("ghost_session", O_CREAT | O_EXCL, 0666);
    if (flag == -1)
    {
        char user2Message[] = "Connected to User 1's chat!\n";
        write(1, user2Message, (int)strlen(user2Message));
        who = 2;
    }
    else
    {
        char user1Message[] = "Waiting for User 2 to join...\n";
        write(1, user1Message, (int)strlen(user1Message));
        who = 1;
        close(flag);
    }

    const char *hist_path = (who == 1) ? "user1History.txt" : "user2History.txt";

    {
        int t = open(hist_path, O_TRUNC | O_CREAT | O_RDWR, 0666);
        if (t >= 0)
            close(t);
    }

    int hist_fd = open(hist_path, O_WRONLY | O_APPEND);

    key_t key = 1010;
    int qid = msgget(key, 0666 | IPC_CREAT);

    pid_t child = fork();
    struct message m;

    if (child > 0)
    {
        for (;;)
        {
            int n = (int)read(0, m.msgText, 999);
            if (n < 0)
                continue;
            m.msgText[n] = '\0';

            m.msgType = (who == 1) ? 2 : 1;

            int burn = 0, burn_sec = 0;
            char *visible = NULL;

            if (is_burn_cmd(m.msgText, n))
            {
                burn = 1;
                visible = burn_split(m.msgText, &burn_sec);
            }

            if (strncmp(m.msgText, "EXIT", 4) == 0)
            {
                msgsnd(qid, &m, sizeof(m.msgText), 0);
                kill(child, SIGTERM);
                wait(NULL);

                msgctl(qid, IPC_RMID, NULL);
                unlink("ghost_session");
                unlink("user1History.txt");
                unlink("user2History.txt");
                exit(0);
            }

            write(1, "\033[A\r\033[K", 4);
            write(1, "You: ", 5);

            if (burn)
            {
                write(1, visible, (int)strlen(visible));
            }
            else
            {
                write(1, m.msgText, (int)strlen(m.msgText));
                log_msg(hist_fd, "You: ", m.msgText);
            }

            msgsnd(qid, &m, sizeof(m.msgText), 0);

            if (burn)
            {
                pid_t b = fork();
                if (b == 0)
                {
                    sleep(burn_sec);
                    clear_screen();
                    print_history(hist_path);
                    _exit(0);
                }
            }
        }
    }
    else
    {
        for (;;)
        {
            long want = (who == 1) ? 1 : 2;
            msgrcv(qid, &m, sizeof(m.msgText), want, 0);

            int burn = 0, burn_sec = 0;
            char *visible = NULL;

            if (is_burn_cmd(m.msgText, (int)strlen(m.msgText)))
            {
                burn = 1;
                visible = burn_split(m.msgText, &burn_sec);
            }

            if (strncmp(m.msgText, "EXIT", 4) == 0)
            {
                write(1, "\n[SYSTEM] The other user has left the chat\n", 43);
                kill(getppid(), SIGTERM);
                exit(0);
            }

            write(1, "Stranger: ", 10);

            if (burn)
            {
                write(1, visible, (int)strlen(visible));
            }
            else
            {
                write(1, m.msgText, (int)strlen(m.msgText));
                log_msg(hist_fd, "Stranger: ", m.msgText);
            }

            if (burn)
            {
                pid_t b = fork();
                if (b == 0)
                {
                    sleep(burn_sec);
                    clear_screen();
                    print_history(hist_path);
                    _exit(0);
                }
            }
        }
    }

    return 0;
}