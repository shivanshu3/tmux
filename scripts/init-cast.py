import re
import sys
import pprint

pp = pprint.PrettyPrinter(indent=2)

filePath = sys.argv[1]

with open(filePath, 'r') as file:
	lines = file.readlines()

regex = re.compile(r"([\w\._\-\/]+)\:(\d+)\:(\d+)\:.+cannot initialize a variable of type '(.+?)\'")

changes = []

for line in lines:
	matches = regex.match(line)
	change = {}
	change['filePath'] = matches[1]
	change['line'] = int(matches[2]) - 1
	# change['column'] = int(matches[3]) - 1
	change['type'] = matches[4]
	changes.append(change)

# pp.pprint(changes)

def ApplyChange(change):
	with open(change['filePath'], 'r') as file:
		lines = file.readlines()
	line = lines[change['line']];
	column = line.index('=') + 2
	line = line[:column] + '(' + change['type'] + ') ' + line[column:]
	lines[change['line']] = line
	with open(change['filePath'], 'w') as file:
		file.writelines(lines)

for change in changes:
	ApplyChange(change)

