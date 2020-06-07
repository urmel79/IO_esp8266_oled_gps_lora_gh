#!/bin/bash
while :
do
  for mdfile in *.md; do
    pdffile=${mdfile%.md}.pdf
#     docxfile=${mdfile%.md}.docx
#     texfile=${mdfile%.md}.tex
#     htmlfile=${mdfile%.md}.html
#     epubfile=${mdfile%.md}.epub
#     odtfile=${mdfile%.md}.odt
    if [ $mdfile -nt $pdffile ]; then
      echo $mdfile
      
#       pandoc -s --from markdown+smart --to latex+smart --variable documentclass=scrartcl --variable classoption=oneside --variable classoption=12pt --variable classoption=a4paper --variable classoption=ngerman --template=./macros/template_n.tex --toc --number-sections --default-image-extension=pdf --filter pandoc-citeproc --bibliography ./literature/Projekt_SMI_4.0.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $pdffile
  
			pandoc -s --from markdown+smart --to latex+smart --variable documentclass=scrartcl --variable classoption=oneside --variable classoption=12pt --variable classoption=a4paper --variable classoption=ngerman --template=./tools/macros/template_n.tex --number-sections --default-image-extension=pdf --filter pandoc-citeproc $mdfile -o $pdffile
  
# mit docx-Template
#       pandoc -s --from markdown+smart --to docx+smart --reference-doc ./macros/template_n.docx -H ./macros/template_n.tex --toc --number-sections --default-image-extension=png --bibliography ./literature/Projekt_SMI_4.0.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $docxfile

#       pandoc ./macros/metadata.yaml -S -s --from markdown+hard_line_breaks --to latex --variable documentclass=scrartcl --variable classoption=oneside --variable classoption=12pt --variable classoption=a4paper --variable classoption=ngerman --template=./macros/template_n.tex --number-sections --default-image-extension=pdf --filter pandoc-citeproc --bibliography ./literature/Electrical_Safety_LVD.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $pdffile

#       pandoc -S -s -t latex --variable documentclass=scrartcl --variable classoption=oneside --variable classoption=12pt --variable classoption=a4paper --variable classoption=ngerman -H ./macros/template.tex --number-sections --default-image-extension=pdf --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $pdffile

#       pandoc ./macros/metadata.yaml -S -s -t latex --variable documentclass=scrartcl --variable classoption=oneside --variable classoption=12pt --variable classoption=a4paper --variable classoption=ngerman -H ./macros/template.tex --number-sections --default-image-extension=pdf --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $pdffile

#       pandoc -S -s -t docx -H ./macros/template.tex --number-sections --default-image-extension=png $mdfile -o $docxfile

#       pandoc ./macros/metadata.yaml -S -s -t docx -H ./macros/template.tex --number-sections --default-image-extension=png --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $docxfile

#       pandoc ./macros/metadata.yaml -S -s -t latex --variable documentclass=scrartcl --variable classoption=oneside --variable classoption=12pt --variable classoption=a4paper --variable classoption=ngerman -H ./macros/template.tex --number-sections --default-image-extension=pdf --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $texfile

#       pandoc ./macros/metadata.yaml -S -s -t html --number-sections --default-image-extension=png --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $htmlfile

#       pandoc ./macros/metadata.yaml -S -s -t epub3 -H ./macros/template.tex --number-sections --default-image-extension=png --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $epubfile

#       pandoc ./macros/metadata.yaml -S -s -t odt -H ./macros/template.tex --number-sections --default-image-extension=png --bibliography ./literature/Projekt_Ind_4_CPS-Ref-Arch.bib --csl ./literature/chicago-author-date-de.csl $mdfile -o $odtfile
    fi
  done
  sleep 5
done
