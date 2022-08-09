#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#include <sys/socket.h>
#include <netdb.h>

#define MAX_FQDN_LEN 255

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[1]) };

    socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr =
    {
        .sin_family = PF_INET,
        .sin_port = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    char buffer[256];

    // socket address used to store client address
    struct sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);
    ssize_t recv_len = 0;

    std::cout << "Running echo server...\n" << std::endl;
    char client_address_buf[INET_ADDRSTRLEN];

    bool go_on = true;
    const char command_string[]="exit";
    while (go_on)
    {
        // Read content into buffer from an incoming client.
        recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                            reinterpret_cast<sockaddr *>(&client_address),
                            &client_address_len);

        if (recv_len > 0)
        {
            //1. Дополните реализованный сервер обратным резолвом имени клиента.
            char client_name_buff[MAX_FQDN_LEN + 1] = {0};
            char failed_back_resolve[] = "<Back resolve failed>";
            int retval;
            /* int getnameinfo(const struct sockaddr *restrict addr, socklen_t addrlen,
                                   char *restrict host, socklen_t hostlen,
                                   char *restrict serv, socklen_t servlen, int flags);
            */
            // https://stackoverflow.com/questions/6170219/problem-while-compiling-socket-connect-code-in-c , answer by asveikau
            //retval = getnameinfo((const struct sockaddr *) &client_address,client_address_len,client_name_buff,MAX_FQDN_LEN,NULL,0,NI_DGRAM);
            retval = getnameinfo( reinterpret_cast<const struct sockaddr *>(&client_address),client_address_len,client_name_buff,MAX_FQDN_LEN,NULL,0,NI_DGRAM|NI_IDN);
            buffer[recv_len] = '\0';
            std::cout
                << "Client"
                << " ["
                << ((retval != 0) ? failed_back_resolve : client_name_buff)
                << "]"
                << " with address "
                << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                << ":" << ntohs(client_address.sin_port)
                << " sent datagram "
                << "[length = "
                << recv_len
                << "]:\n'''\n"
                << buffer
                << "\n'''"
                << std::endl;
            // 2. Дополните реализованный сервер так, чтобы он принимал команду “exit” и,
            // при её получении, завершал работу.
            if (recv_len == 5)
            {
                retval = std::strncmp(reinterpret_cast<const char *>(buffer),command_string,4);
                if (retval == 0)
                {
                    std::cout << "Client requested server to quit. Bye.\n";
                    go_on = false;
                }

            }
            // Send same content back to the client ("echo").
            sendto(sock, buffer, recv_len, 0, reinterpret_cast<const sockaddr *>(&client_address),
                   client_address_len);
        }

        std::cout << std::endl;
    }

    sock.close();
    return EXIT_SUCCESS;
}

