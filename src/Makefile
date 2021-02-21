#--------------------
#- Enironment Setup
#--------------------
CC = gcc
CFLAGS = -Wall -O2 -rdynamic -DCOMPILE_PARSE_TREE
UNIX_RM = /bin/rm -f
STRIP = strip
OS = `uname -s`
SETTINGS = "\\n\\tOperating System support:\\t\"$(OS)\""
LIBRARIES =
INCLUDES = 
PREFIX=../../
PGSQLBIN=/usr/bin/psql
BISON=bison -d -v
FLEX=flex -Cr

ifeq ($(DATABASE),)
  DATABASE=MYSQL_DATABASE
endif

#--------------------------
#- Specific make settings
#--------------------------
ifeq ($(DATABASE),PGSQL_DATABASE)
  SETTINGS += "\\n\\tDatabase support:\\t\\t\"PostgreSQL Version: `$(PGSQLBIN) --version | head -n 1`\""  
  INCLUDES += -I$(PREFIX)/include/pgsql
  LIBRARIES += -L/usr/local/pgsql/lib -lpq -lz -lm
  DBSUPPORT = 1
  DHUFISH_BASE     = api.o config.o dbcalls.o dbpgsql.o errors.o env.o file.o groups.o image.o ipc.o \
                 logging.o malloc.o objects.o syntax.o parser.o request.o search.o \
                 dhufish.o strings.o structs.o users.o package.o md5.o base64.o xml.o webdav.o calendar.o board.o \
                 language.tab.o lex.yy.o parserstream.o
endif


ifeq ($(DATABASE),MYSQL_DATABASE)
  DBPREFIX = /usr
  SETTINGS += "\\n\\tDatabase support:\\t\\t\"Native Mysql Version: `$(DBPREFIX)/bin/mariadb --version | cut -d\  -f6 | cut -d, -f1`\""  
  INCLUDES += -I$(DBPREFIX)/include/mysql
  LIBRARIES += -L$(DBPREFIX)/lib/mariadb -lmariadbclient -lz -lm
  DBSUPPORT = 1
  DHUFISH_BASE     = api.o config.o dbcalls.o dbmysql.o errors.o env.o file.o groups.o image.o ipc.o \
                 logging.o malloc.o objects.o syntax.o parser.o request.o search.o \
                 dhufish.o strings.o structs.o users.o package.o md5.o base64.o xml.o webdav.o calendar.o board.o \
                 language.tab.o lex.yy.o parserstream.o 
  CGI_BASE = cgicommon.o dhufishcgi.o env.o ipc.o malloc.o strings.o config.o md5.o
  CGIISP_BASE = cgicommon.c dhufishcgi.c env.c ipc.c malloc.c strings.c config.c md5.c api.c dbcalls.c dbmysql.c errors.c file.c groups.c image.c logging.c objects.c syntax.c parser.c request.c search.c structs.c users.c package.c base64.c xml.c webdav.c calendar.c board.c language.tab.c lex.yy.c parserstream.c
  DAV_BASE = cgicommon.o webdavcgi.o errors.o env.o ipc.o malloc.o strings.o
endif

ifeq ($(DATABASE),MYSQL_EMBEDDED_DATABASE)
  DBPREFIX = /home/damyon/Desktop/Projects/mysql-4.1.13a
  SETTINGS += "\\n\\tDatabase support:\\t\\t\"Embedded Mysql Version: `$(DBPREFIX)/bin/mysql --version | cut -d\  -f6 | cut -d, -f1`\""  
  INCLUDES += -I$(DBPREFIX)/include
  LIBRARIES += -L$(PREFIX)/lib $(DBPREFIX)/lib/libmysqld.a -lz -lm -lpthread -lcrypt
  DBSUPPORT = 1
  DHUFISH_BASE     = api.o config.o dbcalls.o dbmysql.o errors.o env.o file.o groups.o image.o ipc.o \
                 logging.o malloc.o objects.o syntax.o parser.o request.o search.o \
                 dhufish.o strings.o structs.o users.o package.o md5.o base64.o xml.o webdav.o calendar.o board.o \
                 language.tab.o lex.yy.o parserstream.o
endif

ifeq ($(MEDIATEMPLE),TRUE)
  CFLAGS += -DMEDIATEMPLE
endif

# Set the specifics to compile and link the actual components.

CFLAGS += $(INCLUDES) -D$(OS) -D$(DATABASE)

SOURCE_DIR = ./
OBJ_DIR = .
EXE_DIR = .

TARGETS = DHUFISH
.PHONY: $(TARGETS)

all:	DHUFISH

link:	$(TARGETS)

clean: 
	@echo Cleaning Content Management System ...
	@$(UNIX_RM) $(EXE_DIR)/cms
	@$(UNIX_RM) $(EXE_DIR)/cms.cgi
	@$(UNIX_RM) $(EXE_DIR)/cms-isp.cgi
	@$(UNIX_RM) $(EXE_DIR)/dav.cgi
	@$(UNIX_RM) $(OBJ_DIR)/*.o
	@$(UNIX_RM) language.tab.c
	@$(UNIX_RM) lex.yy.c



#-----------------------
#- TARGETS
#-----------------------
DHUFISH:$(EXE_DIR)/cms $(EXE_DIR)/cms.cgi $(EXE_DIR)/dav.cgi $(EXE_DIR)/cms-isp.cgi

language.tab.c:language.y
	$(BISON) language.y

lex.yy.c:language.l
	$(FLEX) language.l

# ---------------------------------------------------------------------
# Link
# ---------------------------------------------------------------------

$(EXE_DIR)/cms:$(DHUFISH_BASE)
	$(CC) $(CFLAGS) -o $(EXE_DIR)/cms $(DHUFISH_BASE) $(LIBRARIES)
	@echo ""
	@echo "Content Management System compiled with the following settings:"
	@echo -e "$(SETTINGS)"
	@echo ""

$(EXE_DIR)/cms.cgi:$(CGI_BASE)
	$(CC) $(CFLAGS) -o $(EXE_DIR)/cms.cgi $(CGI_BASE)
	@echo ""
	@echo "Content Management System CGI compiled with the following settings:"
	@echo -e "$(SETTINGS)"
	@echo ""

$(EXE_DIR)/cms-isp.cgi:$(CGIISP_BASE)
	$(CC) $(CFLAGS) -DISPVERSION $(CGIISP_BASE) -o $(EXE_DIR)/cms-isp.cgi $(LIBRARIES)
	@echo ""
	@echo "Content Management System CGI compiled with the following settings:"
	@echo -e "$(SETTINGS)"
	@echo ""

$(EXE_DIR)/dav.cgi:$(DAV_BASE)
	$(CC) $(CFLAGS) -o $(EXE_DIR)/dav.cgi $(DAV_BASE)
	@echo ""
	@echo "Content Management System Webdav CGI compiled with the following settings:"
	@echo -e "$(SETTINGS)"
	@echo ""
