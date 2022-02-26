# bed6FloatScore.sql was originally generated by the autoSql program, which also 
# generated bed6FloatScore.c and bed6FloatScore.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Really BED 4 +.  Like BED 6, but with floating-point score (not int).
CREATE TABLE bed6FloatScore (
    bin smallint not null,      # Index field
    chrom varchar(255) not null,	# Chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score float not null,	# Floating point score.
    strand char(1) not null,	# + or -
              #Indices
    INDEX(chrom(8),bin),
    INDEX(chrom(8),chromStart),
    INDEX(name(12))
);
