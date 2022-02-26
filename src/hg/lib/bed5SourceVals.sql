# bed5SourceVals.sql was originally generated by the autoSql program, which also 
# generated bed5SourceVals.c and bed5SourceVals.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#BED5+ with a count, list of sources, and list of source scores for combined data
CREATE TABLE bed5SourceVals (
    bin int unsigned not null,	# Bin number for browser speedup
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Display score (0-1000)
    sourceCount int unsigned not null,	# Number of sources
    sourceIds longblob not null,	# Source ids
    sourceScores longblob not null,	# Source scores
              #Indices
    INDEX (chrom,bin)
);
