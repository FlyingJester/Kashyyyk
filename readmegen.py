import os
import sys
import subprocess

gradient_types =[
 "-webkit-linear-gradient(left, ",
 "-o-linear-gradient(     right,",
 "-moz-linear-gradient(   right,",
 "linear-gradient(     to right,"]

gradient_from = "black"
gradient_to = "#222233"

gradient = gradient_from+" 0px, "+gradient_to+"  calc(50% - ( 928px / 2 ) ), "+gradient_to+"  calc( 50% + ( 928px / 2 ) ), "+gradient_from+" 100%);"

css = open("kashyyyk.css", "w")
css.write("body {"
"align: center;"
"margin: 0px auto;"
"width: 50%;"
"color: #888899;"
"font-family: \"DejaVu sans\";\n")

for i in gradient_types:
  css.write("background: ")
  css.write(i)
  css.write(gradient)
  css.write("\n")

css.write("}\n")
css.write("h1 {"
    "box-shadow: 10px 5px 5px #555577;"
    "color: black;"
    "padding-left: 1em;"
    "padding-right: 1em;"
    "padding-top: 0.5em;"
    "padding-bottom: 0.25em;"
    "border-color: #666666;"
    "background: #AAAAAA;"
    "border-radius:8px;"
    "border-color: #220202;"
    "}"
    "\n"
    "h2 {"
    "box-shadow: 5px 2px 2px #555577;"
    "}")

css.close()

def generate():
  try:
    subprocess.call(["pandoc", "--reference-links", "README.md", "--css=kashyyyk.css", "-o", "kashyyyk.html"])
    subprocess.call(["pandoc", "--reference-links", "license.txt", "--css=kashyyyk.css",  "-o", "license.html"])
  except OSError as e:
    print "pandoc was not installed. Not generating HTML readme."

