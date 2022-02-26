# clinvarSub.sql was originally generated by the autoSql program, which also 
# generated clinvarSub.c and clinvarSub.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#ClinVar variant submission info
CREATE TABLE clinvarSub (
    varId int not null,	# the identifier assigned by ClinVar and used to build the URL, namely https://ncbi.nlm.nih.gov/clinvar/VariationID
    clinSign varchar(255) not null,	# interpretation of the variation-condition relationship
    dateLastEval varchar(255) not null,	# the last date the variation-condition relationship was evaluated by this submitter
    description longblob not null,	# an optional free text description of the basis of the interpretation
    subPhenoInfo longblob not null,	# the name(s) or identifier(s)  submitted for the condition that was interpreted relative to the variant
    repPhenoInfo longblob not null,	# the MedGen identifier/name combinations ClinVar uses to report the condition that was interpreted. 'na' means there is no public identifer in MedGen for the condition.
    revStatus varchar(255) not null,	# the level of review for this submission, namely http//www.ncbi.nlm.nih.gov/clinvar/docs/variation_report/#review_status
    collMethod varchar(255) not null,	# the method by which the submitter obtained the information provided
    originCounts varchar(255) not null,	# origin and the number of observations for each origin
    submitter varchar(255) not null,	# the submitter of this record
    scv varchar(255) not null,	# the accession and current version assigned by ClinVar to the submitted interpretation of the variation-condition relationship
    subGeneSymbol varchar(255) not null,	# the gene symbol reported in this record
    explOfInterp longblob not null,	# more details if ClinicalSignificance is 'other' or 'drug response'
              #Indices
    INDEX(varId)
);
