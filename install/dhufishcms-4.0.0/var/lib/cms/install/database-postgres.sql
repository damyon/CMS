--
-- PostgreSQL database dump
--

SET client_encoding = 'SQL_ASCII';
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: SCHEMA public; Type: COMMENT; Schema: -; Owner: postgres
--

COMMENT ON SCHEMA public IS 'Standard public schema';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: dictionary; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE dictionary (
    wordid integer DEFAULT nextval('Dictionary_wordID_seq'::text) NOT NULL,
    wordstr character varying(255) DEFAULT ''::character varying NOT NULL,
    wordmean double precision DEFAULT (0)::double precision NOT NULL,
    wordstd double precision DEFAULT (0)::double precision NOT NULL
);


ALTER TABLE public.dictionary OWNER TO dhufish;

--
-- Name: dictionary_wordid_seq; Type: SEQUENCE; Schema: public; Owner: dhufish
--

CREATE SEQUENCE dictionary_wordid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.dictionary_wordid_seq OWNER TO dhufish;

--
-- Name: dictionary_wordid_seq; Type: SEQUENCE SET; Schema: public; Owner: dhufish
--

SELECT pg_catalog.setval('dictionary_wordid_seq', 1000, true);


--
-- Name: groupmembers; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE groupmembers (
    groupid integer DEFAULT 0 NOT NULL,
    userid integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.groupmembers OWNER TO dhufish;

--
-- Name: groups; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE groups (
    groupid integer DEFAULT nextval('Groups_groupID_seq'::text) NOT NULL,
    groupname character varying(255) DEFAULT ''::character varying NOT NULL,
    ispublic character(1) DEFAULT 'n'::bpchar,
    CONSTRAINT groups_ispublic CHECK (((ispublic = 'y'::bpchar) OR (ispublic = 'n'::bpchar)))
);


ALTER TABLE public.groups OWNER TO dhufish;

--
-- Name: groups_groupid_seq; Type: SEQUENCE; Schema: public; Owner: dhufish
--

CREATE SEQUENCE groups_groupid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.groups_groupid_seq OWNER TO dhufish;

--
-- Name: groups_groupid_seq; Type: SEQUENCE SET; Schema: public; Owner: dhufish
--

SELECT pg_catalog.setval('groups_groupid_seq', 1000, true);


--
-- Name: objectmetadata; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE objectmetadata (
    objectid integer DEFAULT 0 NOT NULL,
    fieldname character varying(255) DEFAULT ''::character varying NOT NULL,
    fieldvalue character varying(255) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.objectmetadata OWNER TO dhufish;

--
-- Name: objects; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE objects (
    objectid integer DEFAULT nextval('Objects_objectID_seq'::text) NOT NULL,
    objectname character varying(255) DEFAULT ''::character varying NOT NULL,
    parentid integer DEFAULT -1 NOT NULL,
    isonline character(1) DEFAULT 'y'::bpchar,
    ispublic character(1) DEFAULT 'n'::bpchar,
    isdeleted character(1) DEFAULT 'n'::bpchar,
    mimetype character varying(64) DEFAULT 'application/unknown'::character varying NOT NULL,
    version integer,
    lockedbyuserid integer DEFAULT -1 NOT NULL,
    "type" character varying(16) DEFAULT 'CONTENT'::character varying,
    "template" text NOT NULL,
    relativeorder integer DEFAULT 50 NOT NULL,
    CONSTRAINT objects_isdeleted CHECK (((isdeleted = 'y'::bpchar) OR (isdeleted = 'n'::bpchar))),
    CONSTRAINT objects_isonline CHECK (((isonline = 'y'::bpchar) OR (isonline = 'n'::bpchar))),
    CONSTRAINT objects_ispublic CHECK (((ispublic = 'y'::bpchar) OR (ispublic = 'n'::bpchar)))
);


ALTER TABLE public.objects OWNER TO dhufish;

--
-- Name: objects_objectid_seq; Type: SEQUENCE; Schema: public; Owner: dhufish
--

CREATE SEQUENCE objects_objectid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.objects_objectid_seq OWNER TO dhufish;

--
-- Name: objects_objectid_seq; Type: SEQUENCE SET; Schema: public; Owner: dhufish
--

SELECT pg_catalog.setval('objects_objectid_seq', 1000, true);


--
-- Name: occurrence; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE occurrence (
    objectid integer DEFAULT 0 NOT NULL,
    wordid integer DEFAULT 0 NOT NULL,
    total integer DEFAULT 1
);


ALTER TABLE public.occurrence OWNER TO dhufish;

--
-- Name: permissions; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE permissions (
    objectid integer DEFAULT 0 NOT NULL,
    groupid integer DEFAULT 0 NOT NULL,
    mask character(3) DEFAULT '---'::bpchar
);


ALTER TABLE public.permissions OWNER TO dhufish;

--
-- Name: sessiondata; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE sessiondata (
    sessionkey character varying(70) DEFAULT ''::character varying NOT NULL,
    fieldname character varying(255) DEFAULT ''::character varying NOT NULL,
    fieldvalue character varying(255) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.sessiondata OWNER TO dhufish;

--
-- Name: sessions; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE sessions (
    sessionkey character varying(70) DEFAULT ''::character varying NOT NULL,
    lastaccess integer DEFAULT 0 NOT NULL,
    username character varying(255) DEFAULT ''::character varying NOT NULL,
    userid integer DEFAULT 0 NOT NULL,
    issuperuser character(1) DEFAULT 'y'::bpchar,
    fullname character varying(255) DEFAULT ''::character varying NOT NULL,
    usertype character varying(255) DEFAULT 'INTERNAL'::character varying NOT NULL,
    CONSTRAINT sessions_issuperuser CHECK (((issuperuser = 'y'::bpchar) OR (issuperuser = 'n'::bpchar)))
);


ALTER TABLE public.sessions OWNER TO dhufish;

--
-- Name: users; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE users (
    userid integer DEFAULT nextval('Users_userID_seq'::text) NOT NULL,
    username character varying(255) DEFAULT ''::character varying NOT NULL,
    "password" character varying(255) DEFAULT ''::character varying NOT NULL,
    isonline character(1) DEFAULT 'y'::bpchar,
    issuperuser character(1) DEFAULT 'n'::bpchar,
    isdeleted character(1) DEFAULT 'n'::bpchar,
    revision integer DEFAULT 0 NOT NULL,
    fullname character varying(255) DEFAULT ''::character varying NOT NULL,
    usertype character varying(255) DEFAULT 'INTERNAL'::character varying NOT NULL,
    email character varying(255) DEFAULT ''::character varying NOT NULL,
    CONSTRAINT users_isdeleted CHECK (((isdeleted = 'y'::bpchar) OR (isdeleted = 'n'::bpchar))),
    CONSTRAINT users_isonline CHECK (((isonline = 'y'::bpchar) OR (isonline = 'n'::bpchar))),
    CONSTRAINT users_issuperuser CHECK (((issuperuser = 'y'::bpchar) OR (issuperuser = 'n'::bpchar)))
);


ALTER TABLE public.users OWNER TO dhufish;

--
-- Name: users_userid_seq; Type: SEQUENCE; Schema: public; Owner: dhufish
--

CREATE SEQUENCE users_userid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.users_userid_seq OWNER TO dhufish;

--
-- Name: users_userid_seq; Type: SEQUENCE SET; Schema: public; Owner: dhufish
--

SELECT pg_catalog.setval('users_userid_seq', 1000, true);


--
-- Name: verifierinstance; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE verifierinstance (
    objectid integer DEFAULT 0 NOT NULL,
    userid integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.verifierinstance OWNER TO dhufish;

--
-- Name: verifiers; Type: TABLE; Schema: public; Owner: dhufish; Tablespace: 
--

CREATE TABLE verifiers (
    objectid integer DEFAULT 0 NOT NULL,
    groupid integer DEFAULT 0 NOT NULL,
    requiresall character(1) DEFAULT 'n'::bpchar,
    CONSTRAINT verifiers_requiresall CHECK (((requiresall = 'y'::bpchar) OR (requiresall = 'n'::bpchar)))
);


ALTER TABLE public.verifiers OWNER TO dhufish;

--
-- Data for Name: dictionary; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: groupmembers; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: groups; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: objectmetadata; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: objects; Type: TABLE DATA; Schema: public; Owner: dhufish
--

INSERT INTO objects (objectid, objectname, parentid, isonline, ispublic, isdeleted, mimetype, version, lockedbyuserid, "type", "template", relativeorder) VALUES (1, 'system', -1, 'y', 'y', 'n', 'application/folder', 1110787619, -1, 'FOLDER', '', 50);
INSERT INTO objects (objectid, objectname, parentid, isonline, ispublic, isdeleted, mimetype, version, lockedbyuserid, "type", "template", relativeorder) VALUES (2, 'bootstrap.txt', 1, 'y', 'y', 'n', 'text/plain', 0, -1, 'RESOURCE', '', 50);


--
-- Data for Name: occurrence; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: permissions; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: sessiondata; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: sessions; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: users; Type: TABLE DATA; Schema: public; Owner: dhufish
--

INSERT INTO users (userid, username, "password", isonline, issuperuser, isdeleted, revision, fullname, usertype, email) VALUES (1, 'administrator', '21ee4a06b22461f207219524cc2f9b3a', 'y', 'y', 'n', 0, 'System Administrator', 'INTERNAL', '');


--
-- Data for Name: verifierinstance; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Data for Name: verifiers; Type: TABLE DATA; Schema: public; Owner: dhufish
--



--
-- Name: dictionary_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY dictionary
    ADD CONSTRAINT dictionary_pkey PRIMARY KEY (wordid);


ALTER INDEX public.dictionary_pkey OWNER TO dhufish;

--
-- Name: dictionary_wordstr_key; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY dictionary
    ADD CONSTRAINT dictionary_wordstr_key UNIQUE (wordstr);


ALTER INDEX public.dictionary_wordstr_key OWNER TO dhufish;

--
-- Name: groupmembers_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY groupmembers
    ADD CONSTRAINT groupmembers_pkey PRIMARY KEY (groupid, userid);


ALTER INDEX public.groupmembers_pkey OWNER TO dhufish;

--
-- Name: groups_groupname_key; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY groups
    ADD CONSTRAINT groups_groupname_key UNIQUE (groupname);


ALTER INDEX public.groups_groupname_key OWNER TO dhufish;

--
-- Name: groups_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY groups
    ADD CONSTRAINT groups_pkey PRIMARY KEY (groupid);


ALTER INDEX public.groups_pkey OWNER TO dhufish;

--
-- Name: objectmetadata_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY objectmetadata
    ADD CONSTRAINT objectmetadata_pkey PRIMARY KEY (objectid, fieldname);


ALTER INDEX public.objectmetadata_pkey OWNER TO dhufish;

--
-- Name: objects_objectname_key; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY objects
    ADD CONSTRAINT objects_objectname_key UNIQUE (objectname, parentid, version);


ALTER INDEX public.objects_objectname_key OWNER TO dhufish;

--
-- Name: objects_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY objects
    ADD CONSTRAINT objects_pkey PRIMARY KEY (objectid);


ALTER INDEX public.objects_pkey OWNER TO dhufish;

--
-- Name: occurrence_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY occurrence
    ADD CONSTRAINT occurrence_pkey PRIMARY KEY (objectid, wordid);


ALTER INDEX public.occurrence_pkey OWNER TO dhufish;

--
-- Name: permissions_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY permissions
    ADD CONSTRAINT permissions_pkey PRIMARY KEY (objectid, groupid);


ALTER INDEX public.permissions_pkey OWNER TO dhufish;

--
-- Name: sessiondata_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY sessiondata
    ADD CONSTRAINT sessiondata_pkey PRIMARY KEY (sessionkey, fieldname);


ALTER INDEX public.sessiondata_pkey OWNER TO dhufish;

--
-- Name: sessions_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY sessions
    ADD CONSTRAINT sessions_pkey PRIMARY KEY (sessionkey);


ALTER INDEX public.sessions_pkey OWNER TO dhufish;

--
-- Name: users_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY users
    ADD CONSTRAINT users_pkey PRIMARY KEY (userid);


ALTER INDEX public.users_pkey OWNER TO dhufish;

--
-- Name: users_username_key; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY users
    ADD CONSTRAINT users_username_key UNIQUE (username, revision);


ALTER INDEX public.users_username_key OWNER TO dhufish;

--
-- Name: verifierinstance_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY verifierinstance
    ADD CONSTRAINT verifierinstance_pkey PRIMARY KEY (objectid, userid);


ALTER INDEX public.verifierinstance_pkey OWNER TO dhufish;

--
-- Name: verifiers_pkey; Type: CONSTRAINT; Schema: public; Owner: dhufish; Tablespace: 
--

ALTER TABLE ONLY verifiers
    ADD CONSTRAINT verifiers_pkey PRIMARY KEY (objectid);


ALTER INDEX public.verifiers_pkey OWNER TO dhufish;

--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


-- Now the calendar tables

CREATE TABLE cal_instance (
	calid integer DEFAULT NEXTVAL('cal_instance_calid_seq'::text) NOT NULL,
	objectPath text NOT NULL);

ALTER TABLE ONLY cal_instance
    ADD CONSTRAINT cal_instance_pkey PRIMARY KEY (calid);

ALTER INDEX public.cal_instance_pkey OWNER TO dhufish;

ALTER TABLE ONLY cal_instance ADD CONSTRAINT cal_instance_objectpath_key UNIQUE(objectPath);

ALTER INDEX public.cal_instance_objectpath_key OWNER TO dhufish;

CREATE SEQUENCE cal_instance_calid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

CREATE TABLE cal_events (
	eventid integer DEFAULT NEXTVAL('cal_events_eventid_seq'::text) NOT NULL,
	created timestamp NOT NULL,
	modified timestamp NOT NULL,
	calid integer NOT NULL
);

CREATE TABLE cal_occurrence (
	occurrenceid integer DEFAULT NEXTVAL('cal_occurrence_occurrenceid_seq'::text) NOT NULL,
	eventid integer NOT NULL,
    	summary character varying(255) DEFAULT ''::character varying NOT NULL,
    	location character varying(255) DEFAULT ''::character varying NOT NULL,
    	description text NOT NULL,
	eventdate date NOT NULL,
	created timestamp NOT NULL,
	modified timestamp NOT NULL,
        allday character(1) DEFAULT 'n'::bpchar,
	starttime time,
	endtime time,
	CONSTRAINT cal_occurrence_allday CHECK (((allday = 'y'::bpchar) OR (allday = 'n'::bpchar))));

ALTER TABLE ONLY cal_events
    ADD CONSTRAINT cal_events_pkey PRIMARY KEY (eventid);

ALTER INDEX public.cal_events_pkey OWNER TO dhufish;

ALTER TABLE ONLY cal_occurrence
    ADD CONSTRAINT cal_occurrence_pkey PRIMARY KEY (occurrenceid);

ALTER INDEX public.cal_occurrence_pkey OWNER TO dhufish;

CREATE SEQUENCE cal_occurrence_occurrenceid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

CREATE SEQUENCE cal_events_eventid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

-- Now the board tables

CREATE TABLE board_instance (
        boardid integer DEFAULT NEXTVAL('board_instance_boardid_seq'::text) NOT NULL,
        objectPath text NOT NULL);

ALTER TABLE ONLY board_instance
    ADD CONSTRAINT board_instance_pkey PRIMARY KEY (boardid);

ALTER INDEX public.board_instance_pkey OWNER TO dhufish;

ALTER TABLE ONLY board_instance ADD CONSTRAINT board_instance_objectpath_key UNIQUE(objectPath);

ALTER INDEX public.board_instance_objectpath_key OWNER TO dhufish;

CREATE SEQUENCE board_instance_boardid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

CREATE TABLE board_topics (
	topicid integer DEFAULT NEXTVAL('board_topics_topicid_seq'::text) NOT NULL,
	boardid integer NOT NULL,
	authorid integer NOT NULL,
    	summary character varying(255) DEFAULT ''::character varying NOT NULL,
    	description text NOT NULL,
        sticky character(1) DEFAULT 'n'::bpchar,
        locked character(1) DEFAULT 'n'::bpchar,
	created timestamp NOT NULL,
	modified timestamp NOT NULL,
        CONSTRAINT board_topics_sticky CHECK (((sticky = 'y'::bpchar) OR (sticky = 'n'::bpchar))),
        CONSTRAINT board_topics_locked CHECK (((locked = 'y'::bpchar) OR (locked = 'n'::bpchar))));

CREATE TABLE board_messages (
	messageid integer DEFAULT NEXTVAL('board_messages_messageid_seq'::text) NOT NULL,
	topicid integer NOT NULL,
	authorid integer NOT NULL,
    	description text NOT NULL,
	views integer NOT NULL,
	created timestamp NOT NULL,
	modified timestamp NOT NULL);

ALTER TABLE ONLY board_topics
    ADD CONSTRAINT board_topics_pkey PRIMARY KEY (topicid);

ALTER INDEX public.board_topics_pkey OWNER TO dhufish;

ALTER TABLE ONLY board_messages
    ADD CONSTRAINT board_messages_pkey PRIMARY KEY (messageid);

ALTER INDEX public.board_messages_pkey OWNER TO dhufish;

CREATE SEQUENCE board_topics_topicid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

CREATE SEQUENCE board_messages_messageid_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;
