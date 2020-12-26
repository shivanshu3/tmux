import re
import sys
import pprint

pp = pprint.PrettyPrinter(indent=2)

filePath = sys.argv[1]
oldName = sys.argv[2]
newName = sys.argv[3]

with open(filePath, 'r') as file:
	lines = file.readlines()

regex = re.compile(r"([\w\._\-\/]+)\:(\d+)\:(\d+)")

positions = []

for line in lines:
	matches = regex.match(line)
	position = {}
	position['filePath'] = matches[1]
	position['line'] = int(matches[2]) - 1
	position['column'] = int(matches[3]) - 1
	positions.append(position)

# pp.pprint(positions)

def ApplyChange(change):
	with open(change['filePath'], 'r') as file:
		lines = file.readlines()
	line = lines[change['line']];
	column = change['column']
	line = line[:column] + newName + line[(column+len(oldName)):]
	lines[change['line']] = line
	with open(change['filePath'], 'w') as file:
		file.writelines(lines)

for change in positions:
	ApplyChange(change)

