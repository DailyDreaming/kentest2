CREATE TABLE rnaStruct (
    chrom varchar(255) not null,	# Chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    size int unsigned not null,	# Size of element.
    secStr longblob not null,	# Parentheses and '.'s which define the secondary structure
    conf longblob # Confidence of secondary-structure annotation per position (0.0-1.0). or empty
);
