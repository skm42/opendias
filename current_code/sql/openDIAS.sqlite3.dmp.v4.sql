BEGIN TRANSACTION;

INSERT INTO tags VALUES(25,'Wages');
UPDATE tags SET tagname='Morgage' WHERE tagid=13;

INSERT INTO config VALUES ("log_verbosity",3);
INSERT INTO config VALUES ("scan_directory","/var/lib/opendias");
INSERT INTO config VALUES ("show_all_available",1);
INSERT INTO config VALUES ("port",8988);
INSERT INTO config VALUES ("log_directory","/var/log/opendias");

CREATE TABLE access_role (
  role INTEGER PRIMARY KEY, 
  rolename char(25) NOT NULL,
  update_access INTEGER NOT NULL,
  view_doc INTEGER NOT NULL,
  edit_doc INTEGER NOT NULL,
  delete_doc INTEGER NOT NULL,
  add_import INTEGER NOT NULL,
  add_scan INTEGER NOT NULL
);

INSERT INTO access_role VALUES (1,"admin",1,1,1,1,1,1);
INSERT INTO access_role VALUES (2,"user",0,1,1,1,1,1);
INSERT INTO access_role VALUES (3,"view",0,1,0,0,0,0);
INSERT INTO access_role VALUES (4,"add",0,0,0,0,1,1);

CREATE TABLE user_access (
  username CHAR(36) NOT NULL,
  password CHAR(36) NOT NULL, 
  role INTEGER NOT NULL
);

CREATE UNIQUE INDEX "usr_acc" on user_access (username ASC);

CREATE TABLE location_access (
  location CHAR(15) NOT NULL, 
  role INTEGER NOT NULL
);

INSERT INTO location_access VALUES ('127.0.0.1', 1);

CREATE UNIQUE INDEX "loc_acc" on location_access (location ASC);

UPDATE version
SET version = 4,
for_app_version = "0.5.8";

COMMIT;

