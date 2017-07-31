PRAGMA page_size = 4096;
PRAGMA foreign_keys = ON;

CREATE TABLE featuretype (
	id              INTEGER PRIMARY KEY,
	name            TEXT    NOT NULL,
	UNIQUE (name ASC));

CREATE TABLE sourcefile (
	id              INTEGER PRIMARY KEY,
	filename        TEXT    NOT NULL,
	UNIQUE (filename ASC));

CREATE TABLE machine (
	id              INTEGER PRIMARY KEY,
	shortname       TEXT    NOT NULL,
	description     TEXT    NOT NULL,
	sourcefile      INTEGER NOT NULL,
	isdevice        INTEGER NOT NULL,
	runnable        INTEGER NOT NULL,
	UNIQUE (shortname ASC),
	UNIQUE (description ASC),
	FOREIGN KEY (sourcefile) REFERENCES sourcefile (id));

CREATE TABLE system (
	id              INTEGER PRIMARY KEY,
	year            TEXT    NOT NULL,
	manufacturer    TEXT    NOT NULL,
	FOREIGN KEY (id) REFERENCES machine (id));

CREATE TABLE cloneof (
	id              INTEGER PRIMARY KEY,
	parent          TEXT    NOT NULL,
	FOREIGN KEY (id) REFERENCES machine (id));

CREATE TABLE romof (
	id              INTEGER PRIMARY KEY,
	parent          TEXT    NOT NULL,
	FOREIGN KEY (id) REFERENCES machine (id));

CREATE TABLE devicereference (
	id              INTEGER PRIMARY KEY,
	machine         INTEGER NOT NULL,
	device          TEXT    NOT NULL,
	UNIQUE (machine ASC, device ASC),
	FOREIGN KEY (machine) REFERENCES machine (id));

CREATE TABLE dipswitch (
	id              INTEGER PRIMARY KEY,
	machine         INTEGER NOT NULL,
	isconfig        INTEGER NOT NULL,
	name            TEXT    NOT NULL,
	tag             TEXT    NOT NULL,
	mask            INTEGER NOT NULL,
	--UNIQUE (machine ASC, tag ASC, mask ASC), not necessarily true, need to expose port conditions
	FOREIGN KEY (machine) REFERENCES machine (id));

CREATE TABLE diplocation (
	id              INTEGER PRIMARY KEY,
	dipswitch       INTEGER NOT NULL,
	bit             INTEGER NOT NULL,
	name            TEXT    NOT NULL,
	num             INTEGER NOT NULL,
	inverted        INTEGER NOT NULL,
	UNIQUE (dipswitch ASC, bit ASC),
	FOREIGN KEY (dipswitch) REFERENCES dipswitch (id));

CREATE TABLE dipvalue (
	id              INTEGER PRIMARY KEY,
	dipswitch       INTEGER NOT NULL,
	name            TEXT    NOT NULL,
	value           INTEGER NOT NULL,
	isdefault       INTEGER NOT NULL,
	FOREIGN KEY (dipswitch) REFERENCES dipswitch (id));

CREATE TABLE feature (
	id              INTEGER PRIMARY KEY,
	machine         INTEGER NOT NULL,
	featuretype     INTEGER NOT NULL,
	status          INTEGER NOT NULL,
	overall         INTEGER NOT NULL,
	UNIQUE (machine ASC, featuretype ASC),
	FOREIGN KEY (machine) REFERENCES machine (id),
	FOREIGN KEY (featuretype) REFERENCES featuretype (id));
