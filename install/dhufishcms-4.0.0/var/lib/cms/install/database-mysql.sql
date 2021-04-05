# 
#  TOC entry 13 (OID 26071)
#  Name: Dictionary; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE DATABASE cms;
use cms;

FLUSH PRIVILEGES;

GRANT ALL PRIVILEGES ON cms.* to 'cms'@'localhost' identified by 'DATABASEPASSWORD';

FLUSH PRIVILEGES;

CREATE TABLE Dictionary (
    wordID integer AUTO_INCREMENT PRIMARY KEY NOT NULL,
    wordStr varchar(128) DEFAULT '' UNIQUE NOT NULL,
    wordMean double precision DEFAULT 0 NOT NULL,
    wordStd double precision DEFAULT 0 NOT NULL
);

ALTER TABLE Dictionary AUTO_INCREMENT = 1000;


# 
#  TOC entry 14 (OID 26081)
#  Name: GroupMembers; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE GroupMembers (
    groupID integer DEFAULT 0 NOT NULL,
    userID integer DEFAULT 0 NOT NULL,
    UNIQUE KEY (groupID, userID)
);

# 
#  TOC entry 15 (OID 26089)
#  Name: Groups; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Groups (
    groupID integer PRIMARY KEY AUTO_INCREMENT NOT NULL,
    groupName varchar(128) DEFAULT '' UNIQUE NOT NULL,
    isPublic enum('y', 'n') DEFAULT 'n'
);

ALTER TABLE Groups AUTO_INCREMENT = 1000;

# 
#  TOC entry 16 (OID 26099)
#  Name: ObjectMetadata; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE ObjectMetadata (
    objectID integer DEFAULT 0 NOT NULL,
    fieldName varchar(128) DEFAULT '' NOT NULL,
    fieldValue varchar(128) DEFAULT '' NOT NULL,
    UNIQUE KEY (objectID, fieldName)
);


# 
#  TOC entry 17 (OID 26108)
#  Name: Objects; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Objects (
    objectID integer NOT NULL AUTO_INCREMENT PRIMARY KEY,
    objectName varchar(128) DEFAULT '' NOT NULL,
    parentID integer DEFAULT -1 NOT NULL,
    isOnline enum('y', 'n') DEFAULT 'y',
    isPublic enum('y', 'n') DEFAULT 'n',
    isDeleted enum('y', 'n') DEFAULT 'n',
    mimeType varchar(64) DEFAULT 'application/unknown' NOT NULL,
    version integer,
    lockedByUserID integer DEFAULT -1 NOT NULL,
    type varchar(16) DEFAULT 'CONTENT',
    template text NOT NULL,
    relativeorder integer DEFAULT 0 NOT NULL,
    publisherUserID integer DEFAULT -1 NOT NULL,
    UNIQUE KEY (objectName, parentID, version)
);

ALTER TABLE Objects AUTO_INCREMENT = 1000;


# 
#  TOC entry 18 (OID 26189)
#  Name: Occurrence; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Occurrence (
    objectID integer DEFAULT 0 NOT NULL,
    wordID integer DEFAULT 0 NOT NULL,
    total integer DEFAULT 1,
    UNIQUE KEY (objectID, wordID)
);


# 
#  TOC entry 19 (OID 26196)
#  Name: Permissions; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Permissions (
    objectID integer DEFAULT 0 NOT NULL,
    groupID integer DEFAULT 0 NOT NULL,
    mask char(3) DEFAULT '---',
    UNIQUE KEY(objectID, groupID)
);


# 
#  TOC entry 20 (OID 26203)
#  Name: SessionData; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE SessionData (
    sessionKey varchar(70) DEFAULT '' NOT NULL,
    fieldName varchar(128) DEFAULT '' NOT NULL,
    fieldValue varchar(128) DEFAULT '' NOT NULL,
    UNIQUE KEY(sessionKey, fieldName)
);


# 
#  TOC entry 21 (OID 26210)
#  Name: Sessions; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Sessions (
    sessionKey varchar(70) DEFAULT '' NOT NULL PRIMARY KEY,
    lastAccess integer DEFAULT 0 NOT NULL,
    userName varchar(128) DEFAULT '' NOT NULL,
    userID integer DEFAULT 0 NOT NULL,
    isSuperUser enum('y', 'n') DEFAULT 'y',
    fullName varchar(128) DEFAULT '' NOT NULL,
    userType varchar(128) DEFAULT 'INTERNAL' NOT NULL
);


# 
#  TOC entry 22 (OID 26223)
#  Name: Users; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Users (
    userID integer AUTO_INCREMENT PRIMARY KEY NOT NULL,
    userName varchar(128) DEFAULT '' NOT NULL,
    password varchar(128) DEFAULT '' NOT NULL,
    isOnline enum('y', 'n') DEFAULT 'y',
    isSuperUser enum('y', 'n') DEFAULT 'n',
    isDeleted enum('y', 'n') DEFAULT 'n',
    revision integer DEFAULT 0 NOT NULL,
    fullName varchar(128) DEFAULT '' NOT NULL,
    userType varchar(128) DEFAULT 'INTERNAL' NOT NULL,
    email varchar(128) DEFAULT '' NOT NULL,
    UNIQUE KEY(userName, revision)
);

ALTER TABLE Users AUTO_INCREMENT = 1000;

# 
#  TOC entry 23 (OID 26241)
#  Name: VerifierInstance; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE VerifierInstance (
    objectID integer DEFAULT 0 NOT NULL,
    userID integer DEFAULT 0 NOT NULL,
    UNIQUE KEY (objectID, userID)
);


# 
#  TOC entry 24 (OID 26247)
#  Name: Verifiers; Type: TABLE; Schema: public; Owner: dhufish
# 

CREATE TABLE Verifiers (
    objectID integer DEFAULT 0 NOT NULL PRIMARY KEY,
    groupID integer DEFAULT 0 NOT NULL,
    requiresAll enum('y', 'n') DEFAULT 'n'
);


--
-- Data for Name: ObjectMetadata; Type: TABLE DATA; Schema: public; Owner: dhufish
--

--
-- Data for Name: Objects; Type: TABLE DATA; Schema: public; Owner: dhufish
--

INSERT INTO Objects (objectID, objectName, parentID, isOnline, isPublic, isDeleted, mimeType, version, lockedByUserID, type, template, publisherUserID) VALUES (1, 'system', -1, 'y', 'y', 'n', 'application/folder', 1110787619, -1, 'FOLDER', '', 1);
INSERT INTO Objects (objectID, objectName, parentID, isOnline, isPublic, isDeleted, mimeType, version, lockedByUserID, type, template, publisherUserID) VALUES (2, 'bootstrap.txt', 1, 'y', 'y', 'n', 'text/plain', 1110787619, -1, 'RESOURCE', '', 1);

--
-- Data for Name: Users; Type: TABLE DATA; Schema: public; Owner: dhufish
--

INSERT INTO Users (userID, userName, password, isOnline, isSuperUser, isDeleted, revision, fullName, userType, email) VALUES (1, 'administrator', '21ee4a06b22461f207219524cc2f9b3a', 'y', 'y', 'n', 0, 'Administrator', 'INTERNAL', '');

CREATE TABLE cal_instance (
	calid integer NOT NULL PRIMARY KEY AUTO_INCREMENT,
	objectPath text NOT NULL);

CREATE TABLE cal_events (
	calid integer NOT NULL,
	eventid integer NOT NULL PRIMARY KEY AUTO_INCREMENT,
	created timestamp NOT NULL,
	modified timestamp NOT NULL
	);

CREATE TABLE cal_occurrence (
	eventid integer NOT NULL,
	occurrenceid integer NOT NULL PRIMARY KEY AUTO_INCREMENT,
	summary varchar(128) NOT NULL,
	location varchar(128) NOT NULL,
	description text NOT NULL,
	eventdate date NOT NULL,
	starttime time NOT NULL,
	endtime time NOT NULL,
	created timestamp NOT NULL,
	modified timestamp NOT NULL,
        allday enum('y', 'n') DEFAULT 'n'
	);

CREATE TABLE board_instance (
	boardid integer NOT NULL PRIMARY KEY AUTO_INCREMENT,
	objectPath text NOT NULL);

CREATE TABLE board_topics (
	topicid integer NOT NULL PRIMARY KEY AUTO_INCREMENT,
	boardid integer NOT NULL,
	authorid integer NOT NULL,
	summary varchar(128) NOT NULL,
	description text NOT NULL,
        locked enum('y', 'n') DEFAULT 'n',
        sticky enum('y', 'n') DEFAULT 'n',
	views integer NOT NULL,
	created timestamp NOT NULL,
	modified timestamp NOT NULL
	);

CREATE TABLE board_messages (
	messageid integer NOT NULL PRIMARY KEY AUTO_INCREMENT,
	topicid integer NOT NULL,
	authorid integer NOT NULL,
	description text NOT NULL,
	created timestamp NOT NULL,
	modified timestamp NOT NULL
	);

CREATE TABLE Notifications (
	objectid integer NOT NULL,
	groupid integer NOT NULL,
	PRIMARY KEY (objectid, groupid)
);
