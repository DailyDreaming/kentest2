# hubSearchText.sql was originally generated by the autoSql program, which also 
# generated hubSearchText.c and hubSearchText.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Track hub descriptions
CREATE TABLE hubSearchText (
    hubUrl longblob,	# Hub URL
    db varchar(255),	# Assembly name (UCSC format) for Assembly and Track descriptions, NULL for hub descriptions
    track varchar(255),	# Track name for track descriptions, NULL for others
    label varchar(255),	# Name to display in search results
    textLength enum("Short", "Long", "Meta"),	# Length of text (short for labels, long for description pages, meta for metadata)
    text longtext,	# Description text
    parents longblob,	# Comma separated list of parent track of this track, NULL for others
    parentTypes longblob, # Comma separated list of parent track types
              #Indices
    KEY (textLength),
    INDEX (track),
    FULLTEXT INDEX (text)
);
