# vax004Maf.sql was originally generated by the autoSql program, which also 
# generated vax004Maf.c and vax004Maf.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#VAX004 HIV-1 DNA Multiple Sequence Alignment
CREATE TABLE vax004Maf (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome (forward strand)
    chromEnd int unsigned not null,	# End position in chromosome
    extFile int unsigned not null,	# Pointer to associated external file
    `offset` bigint not null,	# Offset in external file
    score float not null,	# Overall score
              #Indices
    PRIMARY KEY(chrom)
);
