# $Id$
# convert head of arx to arguments of rvp
/grammars/,/}/ {
  s/[ 	]+/ /g
  s/#.*$|(^| )grammars( |{|$)|{|}/ /g
  s/ *([^ ]+)="([^ ]+)" */\1=\2 /gp
}
