#include <bits/stdc++.h>
#define sz(a) ((int)((a).size()))
//#define char unsigned char
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
/*
 *
 * */
 
int x;
int index_current_node, index_next_node;
std::map <int,int> hashTable;
const int N = 6;
struct node {
    std::string ip_str;
  int ip_int;
  int port_tcp;
  int port_udp;
    node() {};
    node(int tcp, int udp) : port_tcp(tcp), port_udp(udp) {}

};

node nodes[N];
const std::string IP = "127.0.0.1";
void init_nodes() {
    int port = 2000;
    for (int i = 0; i < N; i++) {
        nodes[i].port_tcp = port++;
        nodes[i].port_udp = port++;
        nodes[i].ip_str = IP;
        std::cout << " index: " << i << " port tcp " << nodes[i].port_tcp << " port udp " << nodes[i].port_udp << '\n'; 
    }
}

std::pair <node, node> select_current_and_next_node(int port_udp, int port_tcp, int port_udp_next){
    index_current_node = 0;
    index_next_node = 1;
    for (int i = 0; i < N; i++) {
        node n1 = nodes[i];
        if (n1.port_tcp == port_tcp && n1.port_udp == port_udp) {
            node n2 = nodes[(i+1) % N];
            if (n2.port_udp == port_udp_next) {
                index_current_node = i;
                index_next_node = (i + 1) % N;
               return std::make_pair(n1,n2); 
            }
        }
    }
    return std::make_pair(node(0,0),node(1,1));
}

void init_hashTable() {
    for (int i = 0; i <= N; i++) {
        hashTable[i] = 0;
    }
}

int get_node_key(int x) {
    return (x % N);
}

int read_value(std::string val) {
     try {
        int num = std::stoi(val);
        return num;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: The string cannot be converted to an int." << std::endl;
        exit(1);
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range: The string is too large to fit in an int." << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        exit(1);
    }
}

bool isCurrentNode(int node_number, node &curr) {
    node nd = nodes[node_number];
    //std::cout << " udp node number es " << nd.port_udp << '\n';
    if (nd.port_udp == curr.port_udp && nd.port_tcp == curr.port_tcp) {
        return true;
    }
    return false;
}

int getCurrentIndexNode(node &curr) {
    for (int i = 0; i < N; i++) {
        if (isCurrentNode(i, curr)) return i;
    }
    return 0;
}

std::string PUT_FORWARD = "PUT_FORWARD";
std::string WHAT_X = "WHAT_X";
std::string PUT_REPLY_X = "PUT_REPLY_X";
std::string GET_FORWARD = "GET_FORWARD";
std::string GET_REPLY_X = "GET_REPLY_X";


void sendUDP(node &next, std::string msg_to_send) {
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        exit(1);
    }

    // Set up the destination address
    int port = next.port_udp;
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port); // Port number
    inet_pton(AF_INET, next.ip_str.c_str(), &dest_addr.sin_addr); // IP address

    
    // Send the message
    ssize_t sent_bytes = sendto(sockfd, msg_to_send.c_str(), msg_to_send.size(), 0,
                                (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        std::cerr << "Error sending message." << std::endl;
        close(sockfd);
        exit(1);
    }

    std::cout << "Message sent successfully!" << std::endl;

    // Close the socket
    close(sockfd);
}

std::vector<std::string> extractFromMsg(char *buffer, size_t buffer_size) {
    std::vector<std::string> message_parts;
    std::istringstream iss(buffer);
    std::string part;

    while (iss >> part) {
        message_parts.push_back(part);
    }

    return message_parts;
}


bool isForActualNode(std::string key, node curr) {
    int node_number = get_node_key(read_value(key));

    for (int i = 0; i < N; i++) {
        node nd = nodes[i];

        if (curr.port_udp == nd.port_udp && nd.port_tcp == curr.port_tcp) {
            if (node_number == i) {
                x = hashTable[i];
            }
            return node_number == i;
        }
    }
    return false;

}


bool send_as_x(const std::string& ip, int port, char symbol, node curr) {
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return false;
    }

    // Set up the destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, ip.c_str(), &dest_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address format." << std::endl;
        close(sockfd);
        return false;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Error connecting to the server." << std::endl;
        close(sockfd);
        return false;
    }
    if (symbol == 'P') {
        // Send the message
        std::string message = WHAT_X + " ";
    
        message += "P";
    
        if (send(sockfd, message.c_str(), message.size(), 0) < 0) {
            std::cerr << "Error sending message." << std::endl;
            close(sockfd);
            return false;
        }
    

        // Read the message
        char buffer[5000];
        ssize_t received_bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (received_bytes < 0) {
            std::cerr << "Error receiving message." << std::endl;
            close(sockfd);
            exit(1);
        }
        // Null-terminate the received data to make it a valid C-string
        buffer[received_bytes] = '\0';
        std::cout << "Received message: " << buffer << std::endl;
        // Close the client socket
        //close(new_socket);

        std::vector<std::string> received_message = extractFromMsg(buffer, sizeof(buffer));
    
        if (sz(received_message) <= 1) {
            std::cerr << "Error message\n ";
            close(sockfd);
            exit(1);
        }
    
        if (received_message[0] == PUT_REPLY_X) {
            // put reply
            x = read_value(received_message[1]);
            int index_node = getCurrentIndexNode(curr);
            hashTable[index_node] = x;
            std::cout << "Value of X save it in hashtable node " << index_node << " val: " << x << '\n';
        }
    }
    else {
        int index_node = getCurrentIndexNode(curr);
        x = hashTable[index_node];
        // Send the message
        std::string message = GET_REPLY_X + " ";

        message += std::to_string(index_node);
        message += " ";
        message += std::to_string(x);
        if (send(sockfd, message.c_str(), message.size(), 0) < 0) {
            std::cerr << "Error sending message." << std::endl;
            close(sockfd);
            return false;
        }
    }


    //std::cout << "Message sent successfully!" << std::endl;
    /*
     *Aqui debo leer el reply de mi socket anterior, cuando le envio el what_x, recibo una respuesta
     que seria el valor de x si es put, y lo guardo en el hastablae
     *
     * */
    // Close the socket
    close(sockfd);
    return true;
}

int32_t main(int argc,char *argv[]) {
    int tcp_fd,udp_fd;
    struct sockaddr_in address, server_addr_udp, client_addr;// viene del include netinet/in.h
    int opt = 1;
    int addrlen1, addrlen2;
    char buffer[1025], buffer_reply[1025];
    fd_set readfd;
    init_nodes();
    init_hashTable();
    if (argc < 4) {
    
        printf("You have to pass 4 arguments to init the code like: ip_address own_udp_port next_udp_port own_tcp_port");
        return 1;
    }

    int port_udp = 0, port_tcp = 0, next_port_udp = 0;
    for (int i = 2; i < argc; i++) {
        try {
            if (i == 2) {
                port_udp = std::stoi(argv[i]);
            }
            if (i == 3) {
                next_port_udp = std::stoi(argv[i]);
            }
            if (i == 4) {
                port_tcp = std::stoi(argv[i]);
            }
            std::cout << argv[i] << '\n';
        } catch (std::invalid_argument const &e) {
            printf("Bad input: std::invalid_argument thrown\n");
            return 1;
        }
    }

    int min_port = std::min(port_udp, port_tcp);
    min_port = std::min(min_port, next_port_udp);

    if (min_port < 2000 || min_port > 2012) {
        printf("Error port unknow");
        return 1;
    }

    std::pair<node, node> current_nodes = select_current_and_next_node(port_udp, port_tcp, next_port_udp);
    //std::cout << " current nodes are " << current_nodes.first.port_udp << "  next " << current_nodes.second.port_udp << '\n';
    //POner los mensajes y sus tipos para ir haciendo cada caso y hacer la hash funtion y hash table
    
    // creando udp y tcp socket
    system("clear");
	//printf ("       INSTRUCTIONS \n\n  =================NODE %d=======================\n",num);
	puts("   1.'put' request format : PUT <integer> <integer>\n");
	puts("   2.'get' request format : GET <integer>\n");
	//puts("   3.To print Hash Table : 'r'\n");
	puts("-----------------------------------\n\nENTER GET/PUT REQUEST :");
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("fallo creacion socket udp");
        exit(1);
    }
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        perror("Fallo creacion socket tcp");
        exit(1);
    }
    // extract data from nodes
    node current = current_nodes.first;
    node next = current_nodes.second;
    //int port_udp = current.port_udp;
    //int port_tcp = current.port_tcp;
    //int next_port_udp = next.port_udp;
    server_addr_udp.sin_family = AF_INET;
    server_addr_udp.sin_port = htons(port_udp);
    server_addr_udp.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr_udp.sin_zero),8);

    // bind port udp
    if (bind(udp_fd, (struct sockaddr *) &server_addr_udp, sizeof(struct sockaddr)) == -1)  {
        perror("Error binding udp\n");
        exit(1);
    }
    addrlen1 = sizeof(struct sockaddr);

    // connect to tcp socket
    // allow reconnection to tcp socket
    if (setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
    }

    address.sin_family = AF_INET;
  	address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_tcp);

    if (bind(tcp_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("Error bind tcp");
        exit(1);
    }
    // listen for this port and tcp socket
    if (listen(tcp_fd, 5)<0)
	{
    		perror("listen");
    		exit(1);
  	}

    printf("All ok\n");
    int max_fd = 0;
    while(true) {
        // Initialize the read fd to zero
        FD_ZERO(&readfd);
        
        FD_SET(udp_fd, &readfd);

        FD_SET(tcp_fd, &readfd);

        FD_SET(0,&readfd); 			// to read from standard input

        max_fd = std::max(udp_fd, tcp_fd);

        select(max_fd+1, &readfd, NULL, NULL, NULL); // call the select method

        // working with the console
        if  (FD_ISSET(0, &readfd))	//FD_ISSET()Returns a non-zero value if the bit for the file descriptor
        {
            std::string line;
            //std::getline(std::cin, line); // Read the whole line into a string
            std::cin >> line;
            //std::cout << line << '\n';            
            //std::cout << "este es el str " << read_data << std::endl;
            if (line == "r" || line== "R") {
                // print hastable get request to all nodes for their values
                printf("print hash table with the values");

            }
            else {
                
                /*if (words.size() <= 1) {
                    perror("Invalid format string is not a put or a get request\n");
                    exit(1);
                }*/
                // put request
                if (line == "PUT" || line == "put") {
                    std::string val1,val2;
                    std::cin >> val1 >> val2;
                    std::cout << val1 << " " << val2 << '\n';
                    int node_number = get_node_key(read_value(val1));
                    x = read_value(val2);
                    if (isCurrentNode(node_number, current_nodes.first)) {
                        printf("current node\n");
                        hashTable[node_number] = x;

                    }
                    else {
                        // forward the request to the next node.
                        // Prepare the message
                    
                        //int port_tcp = current_nodes.first.port_tcp;
                        std::ostringstream msg_stream;
                        msg_stream << PUT_FORWARD << " " << val1 << " " << current_nodes.first.ip_str  << " " << port_tcp;
                        std::string msg_to_send = msg_stream.str();

                        sendUDP(current_nodes.second, msg_to_send);

                    }
                    
                }
                // get request
                else if (line == "GET" || line == "get") {
                    std::string val1;
                    std::cin >> val1;
                    int node_number = get_node_key(read_value(val1));

                    if (isCurrentNode(node_number, current_nodes.first)) {
                        printf("current node\n");
                        //hashTable[node_number] = x;
                        std::cout << hashTable[node_number] << '\n';

                    }
                    else {
                        // forward the request to the next node.
                        // Prepare the message
                    
                        //int port_tcp = current_nodes.first.port_tcp;
                        std::ostringstream msg_stream;
                        msg_stream << GET_FORWARD << " " << val1 << " " << current_nodes.first.ip_str  << " " << port_tcp;
                        std::string msg_to_send = msg_stream.str();

                        sendUDP(current_nodes.second, msg_to_send);

                    }


                }
                else {
                    perror("Invalid format\n");
                    exit(1);
                }

            }
        }
        if (FD_ISSET(udp_fd, &readfd)) {
            // receive comunication by udp
            char rec_buff[5000];
            socklen_t sender_addr_len = sizeof(client_addr);
            int len = recvfrom(udp_fd, rec_buff, 5000, 0, (struct sockaddr *)&client_addr, &sender_addr_len);
               if (len < 0) {
                std::cerr << "Error receiving message." << std::endl;
                exit(1);
            }

            rec_buff[len] = '\0'; // end of the string
            printf("%s\n",rec_buff);
            
            std::vector<std::string> received_message = extractFromMsg(rec_buff, sizeof(rec_buff));

            if (sz(received_message) < 1) {
                perror("Error message received\n");
                exit(1);
            }
            // the key is in the second item
            std::string key = received_message[1];
            if (!isForActualNode(key, current_nodes.first)) {
                // is not actual node, forward the message
                std::string msg_to_send = "";
                for (int i = 0; i < sz(received_message); i++) {
                    msg_to_send += received_message[i];

                    if (i == sz(received_message) - 1) {
                        continue;
                        //msg_to_send += "\0";
                    }
                    else {
                        msg_to_send += " ";
                    }
                }

                sendUDP(current_nodes.second, msg_to_send);
                puts("\n---------------------------------------\nENTER NEW GET/PUT REQUEST:");


            }
            else {
                // is for me, check the content of the message if is a put or a get
                // and do something depends of what is
                printf("Received message to me\n");

                if (received_message[0] == PUT_FORWARD) {
                    // is from a put request, send tcp message to originator asking for what_x
                    if (!send_as_x(received_message[2], read_value(received_message[3]),'P', 
                                current_nodes.first)) {
                        perror("Error send as x\n");
                        exit(1);
                    }
                   // printf("send as x\n");
                }
                else {
                    // is from a get request
                    if (!send_as_x(received_message[2], read_value(received_message[3]), 'G',
                                    current_nodes.first)) {
                        
                        perror("Error send get_reply x\n");
                        exit(1);
                    }
                }
            }

        }
        if (FD_ISSET(tcp_fd, &readfd)) {
            // connection from tcp. You can received what_x, put_reply_x, or get_reply_x
            //
            socklen_t addr_len=sizeof(address);
            int new_socket;
			if ((new_socket = accept(tcp_fd, (struct sockaddr *)&address, &addr_len))<0)
			{
				/* if accept failed to return a socket descriptor, display error and exit */
        		perror("accept");
        		exit(1);
       		}
            // Read the message
            char buffer[5000];
            ssize_t received_bytes = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
            if (received_bytes < 0) {
                std::cerr << "Error receiving message." << std::endl;
                close(new_socket);
                return 1;
            }
            // Null-terminate the received data to make it a valid C-string
            buffer[received_bytes] = '\0';
            std::cout << "Received message: " << buffer << std::endl;
            // Close the client socket
            //close(new_socket);

            std::vector<std::string> received_message = extractFromMsg(buffer, sizeof(buffer));

            if (sz(received_message) < 1) {
                perror("Error message received\n");
                exit(1);
            }
            std::cout << received_message[0] << " received " << received_message[1] << '\n'; 
            // check if the message received is for put or get reply (what_x)
            if (received_message[0] == WHAT_X) {
                std::string message_reply = "";
                if (received_message[1] == "P") {
                    // what for a put request, send value of x
                    message_reply = PUT_REPLY_X;
                    printf("what x para put\n");
                }
                else {
                    // is for a get request
                    message_reply = GET_REPLY_X;
                    printf("what x para get\n");
                }
                message_reply += " ";
                message_reply += std::to_string(x);
                const char * msg_reply = message_reply.c_str();
                if (send(new_socket, msg_reply, strlen(msg_reply), 0) == -1 ) {
                    perror("Error al send");
                    exit(1);
                }
                printf("message send %s\n", msg_reply);
                //read(new_socket, buffer, 1024);
            	//printf("\n%s\n",buffer);
                //close(new_socket);

            }
            else {
                //int val = read_value(received_message[1]);
                std::cout << " Node: " << received_message[1] << " value: " << received_message[2] << '\n';
            }
            
            close(new_socket);

        }

    }
    return 0;
}
