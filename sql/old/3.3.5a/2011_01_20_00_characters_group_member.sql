ALTER TABLE `group_member`
ROW_FORMAT=DEFAULT,
CHANGE `guid` `guid` INT(10) UNSIGNED NOT NULL,
CHANGE `memberGuid` `memberGuid` INT(10) UNSIGNED NOT NULL;