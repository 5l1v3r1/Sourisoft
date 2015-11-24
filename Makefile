CC			= g++
CFLAGS		= -g -Wall
INCLUDES	= 
LDFLAGS		= 
LIBS		= 
OBJS1		= client.o network.o
OBJS2		= server.o network.o
PROGRAM1	= client 
PROGRAM2	= server 

$(PROGRAM1):	$(OBJS1) server
				$(CC) $(LDFLAGS) $(LIBS) $(OBJS1) -o $(PROGRAM1)

server:			$(OBJS2)
				$(CC) $(LDFLAGS) $(LIBS) $(OBJS2) -o $(PROGRAM2)

clean:;		rm -f *.o *~ $(PROGRAM1) $(PROGRAM2)
