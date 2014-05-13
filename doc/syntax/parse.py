#!/usr/bin/python

#% Production name
#\newcommand{\pn}[1]{\langle \textnormal{#1} \rangle}
#% Produces
#\newcommand{\pp}{\rightarrow}
#% Or
#\newcommand{\oo}{\; \mid \;}
#% Dots
#\newcommand{\sk}{\dots }
#% Space
#\newcommand{\ww}{\;}
#% Null
#\newcommand{\nn}{\perp}
#% Symbol terminal
#\newcommand{\sm}[1]{\textnormal{\bf #1}}
#% Descriptive terminal
#\newcommand{\sd}[1]{\textnormal{\it #1}}
#% production rule 
#\newcommand{\pr}[1]{\hspace{5mm}\sd{#1}}

import sys
import re
import itertools

HEADER  = '\033[95m'
BOLD    = "\033[1m"
BLUE    = '\033[94m'
GREEN   = '\033[92m'
WARNING = '\033[93m'
FAIL    = '\033[91m'
ENDC    = '\033[0m'

DEBUG = 0

def esc(s):
  s = re.sub('{', '\\{', s)
  s = re.sub('}', '\\}', s)
  s = re.sub('&', '\\&', s)
  s = re.sub('#', '\\#', s)
  return s
    
scanner=re.Scanner([
  (r'[A-Za-z\-]+',  lambda scanner,token:('PROD', token)),
  (r'\"[^\"]+\"',   lambda scanner,token:('TERM', token)),
  (r'<[^\"<]+>',    lambda scanner,token:('SYM', token)),
  (r'\{',           lambda scanner,token:('BLIST', token)),
  (r'\}',           lambda scanner,token:('ELIST', token)),
  (r'[0-9]+',       lambda scanner,token:('INT', token)),
  (r'=',            lambda scanner,token:('DEF', token)),
  (r'\|',           lambda scanner,token:('OR', token)),
  (r'\s+', None),
  ])

def printRule(r):
  results, rem = scanner.scan(r)
  if DEBUG:
    print BLUE+'{}'.format(results)+ENDC 
  i = 0
  while i < len(results):
    (t, x) = results[i]
    i += 1

    # Production name
    if t == 'PROD':
      print '\pr{'+esc(x)+'}\ww',

    # Terminal
    elif t == 'TERM':
      print '\sm{'+esc(x[1:-1])+'}\ww',

    # Symbol
    elif t == 'SYM':
      print '\pn{'+esc(x[1:-1])+'}\ww',

    # Begin list
    elif t == 'BLIST':
      (tn1, xn1) = results[i]
      assert tn1 == 'INT'
      i += 1
      (tn2, xn2) = results[i]
      if tn2 == 'TERM':
        i += 1
        print esc(x)+'_{'+xn1+'}\ww\sm{'+esc(xn2[1:-1])+'}\ww',
      else:
        # If it wasn't a separator, stick it back
        print esc(x)+'_{'+xn1+'}\ww',
        i -= 1

    # Begin list
    elif t == 'ELIST':
      print esc(x)+'\ww',

    # Produces
    elif t == 'DEF':
      print '\pp & \ww',

    # Or
    elif t == 'OR':
      print '\\\\\n\oo & \ww',
    
def getKeywordsFromRule(r):
  results, rem = scanner.scan(r)
  if DEBUG:
    print BLUE+'{}'.format(results)+ENDC 
  i = 0
  keywords = []
  while i < len(results):
    (t, x) = results[i]
    i += 1
    # Terminal
    if t == 'TERM':
      terminal = esc(x[1:-1]) 
      if re.match('^[\w-]+$', terminal) is not None:
        keywords.append(terminal)
  return keywords

def printCollected(sections):
  # Parse each section and print collected syntax
  for (title, level, contents) in sections:

    # Split on the rule separator
    rules = contents.split(';\n')
    # Remove any empty rules (sometimes produced at the end from ws)
    rules = [x for x in rules if len(x)>0]

    if level == 1:
      print('\n\n\\subsection{'+title+'}\n')
    if level == 2:
      print('\n\\subsubsection{'+title+'}\n')

    if len(rules) > 0: 
      print('\\begin{flalign*}')

      # Print each rule
      for (index, r) in enumerate(rules):
        # Remove remaining newlines
        #r = re.sub('\n', '', r)
        if DEBUG:
          print GREEN+r+ENDC
        
        printRule(r)

        # Add LaTeX newlines after all but the last rule
        if (index < len(rules)-1):
          print '&\\\\\n',
        else:
          print '&\n',
          
      print('\\end{flalign*}')


def printOrdered(sections):
  # Combine rules
  combined = {}
  for (i, (title, level, contents)) in enumerate(sections):
    productions = contents.split(';\n')
    for x in productions:
      if x != '\n' and x != '':
        #print('='*20"\n{}: {}\n"+'='*20.format(i, x))
        split = x.split('= ')
        # Get the name of the production
        name = split[0].strip()
        if not name in combined:
          combined[name] = []
        #print(name)
        # Get the alternatives and add then to the name
        alternatives = split[1].split('| ')
        for x in alternatives:
          combined[name].append(x)
          #print("  "+x)

  # Print ordered syntax
  for x in sorted(combined.keys()):
    print('\\begin{flalign*}')
    if len(combined[x]) > 0: 
      print("\pr{"+x+"} \pp & \ww")

      # Print each rule
      for (i, y) in enumerate(sorted(combined[x])):
        #print("  "+y)
        printRule(y)

        if i < len(combined[x])-1:
          print("&")
          printRule("|")
        else:
          print '&',
          
    print('\\end{flalign*}')


def printKeywords(sections):
  # Combine rules
  keywords = set()
  for (i, (title, level, contents)) in enumerate(sections):
    productions = contents.split(';\n')
    for x in productions:
      if x != '\n' and x != '':
        #print('='*20"\n{}: {}\n"+'='*20.format(i, x))
        split = x.split('= ')
        # Get the name of the production
        #name = split[0].strip()
        #if not name in combined:
        #  combined[name] = []
        #print(name)
        # Get the alternatives and add then to the name
        alternatives = split[1].split('| ')
        for x in alternatives:
          #combined[name].append(x)
          #print("  "+x)
          newKeywords = getKeywordsFromRule(x)
          for y in newKeywords:
            keywords.add(y)
  print('\\begin{multicols}{3}\n\\begin{itemize}')
  for x in sorted(keywords):
    print('\\item[] $\\sm{\\w{'+x+'}}$')
  print('\\end{itemize}\n\\end{multicols}')


f = open('syntax.ebnf', 'r')
section = ''
level = 0
contents = ''
sections = []

# Strip comments and blank lines and divide into sections
# Rules are separated by the string ';\n'
for l in f:
  l = re.sub(r'%[^\n]*', '', l) # Comments
  l = l.strip(' ') # Remove spaces
  if len(l) == 0:
    continue
  if l == '\n':
    continue
  #print 'line: '+l,
  # Extract subsection
  r = re.match(r'\[\[([A-Za-z ]+)\]\]\s*$\n', l)
  if r:
    if level != 0:
      sections.append((section, level, contents))
      #print(section)
      #print(contents)
    section = r.group(1)
    level = 1
    contents = ''
    continue
  # Extract subsubsections
  r = re.match(r'\[([A-Za-z ]+)\]\s*\n', l)
  if r:
    assert level != 0
    sections.append((section, level, contents))
    #print(section)
    #print(contents)
    section = r.group(1)
    level = 2
    contents = ''
    continue
  # Concatenate EBNF
  contents += l
f.close()

if level != 0:
  sections.append((section, level, contents))

if sys.argv[1] == 'collected':
  printCollected(sections)
if sys.argv[1] == 'ordered':
  printOrdered(sections)
if sys.argv[1] == 'keywords':
  printKeywords(sections)

