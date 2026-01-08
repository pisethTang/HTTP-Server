# HTTP-Server

Implementing epoll, sendfile, thread pools and zero-copy I/O (sendfile) 

## Phase 1: 
- Focus: Sockets, File Descriptors, and the life cycle of a packet. 
- Syscalls: socket, bind, listen, accept, read, write, close. 
- Deliverable: A program halts execution, waits for a browser to connect, and dumps the raw HTTP text (headers) to the terminal. 

## Phase 2: The Protocol (HTTP)
- Focus: String parsing without blowing up memory (Buffer Overflows).
- Concepts: HTTP Verbs (GET), Headers (\r\n\r\n), MIME types.
- Deliverable: A server that returns valid HTML. "Hello World" renders in the browser, not just the terminal.

## Phase 3: The Resume Standard (Replicating the Reference)
- Focus: Concurrency (Level 1) and Dynamic Content.
- Syscalls: fork, execve, dup2, pipe, waitpid.
- Concepts: Blocking vs. Non-Blocking I/O (poll).
- Deliverable: Handling multiple clients (poorly, but functioning) and running Python scripts via CGI.

## Phase 4: "God Mode" (High-Performance Architecture)
- Focus: O(1) Event Loops, Context Switching, and Kernel bypass.
- Syscalls: epoll_create, epoll_ctl, epoll_wait, sendfile.
- Concepts: The C10k problem, Edge Triggered vs. Level Triggered events, Thread Pools (avoiding fork overhead).
- Deliverable: A server capable of handling thousands of concurrent connections with minimal CPU usage.