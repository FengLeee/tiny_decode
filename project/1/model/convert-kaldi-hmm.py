# !/usr/bin/python

# convert kaldi hmm to quad form of <phone , hmm-state , pdf , number-of-transition>

f = open('final.mdl' , 'r')

phone_state_trans_num_dict = {}

line = f.readline().strip()
assert '<TransitionModel>' == line

line = f.readline().strip()
assert '<Topology>' == line

while(True) :
	line = f.readline().strip()
	if('</Topology>' == line) : break
	elif('<TopologyEntry>' != line) : assert(False)
	else :
		line2 = f.readline().strip()
		assert '<ForPhones>' == line2

		phones_list = []
		line2 = f.readline()
		start_pos = 0 

		while True :
			pos = line2.find(' ' , start_pos)
			if -1 == pos : break 
			phone = line2[start_pos : pos].strip()
			phones_list.append(phone) 
			start_pos = pos + 1
		
		line2 = f.readline().strip()
		assert '</ForPhones>' == line2
	
		while True :
			line2 = f.readline().strip()
			if '</TopologyEntry>' == line2 : break 

			start_pos = 0 
			pos = line2.find(' ' , start_pos)

			start_pos = pos + 1 
			pos = line2.find(' ' , start_pos)
			state = line2[start_pos : pos]
			
			start_pos = pos + 1
			num_trans = 0 

			while True :
				pos = line2.find('<Transition>' , start_pos)

				if -1 == pos : break
				else : 
					num_trans += 1
					start_pos = pos + len("<Transtion>")
			
			for phone in phones_list :
				phone_state_trans_num_dict[((int)(phone) , (int)(state))] = (int)(num_trans) 

line = f.readline()
pos = line.find(' ' , 0)
pos += 1
end_pos = line.find('\n' , pos)
assert '<Triples>' == line[0 : pos].strip()
num_hmm_state = int(line[pos : end_pos].strip())

print '<HMMTransitionModel>' , num_hmm_state

while True :
	line = f.readline()
	line2 = line.strip()

	if '</Triples>' == line2 : break

	start_pos = 0
	pos = line.find(' ' , start_pos)
	phone = line[start_pos : pos].strip()

	start_pos = pos + 1
	pos = line.find(' ' , start_pos)
	state = line[start_pos : pos].strip()
	
	start_pos = pos + 1
	pos = line.find('\n' , start_pos)
	pdf = line[start_pos : pos + 1].strip()
	
	print phone , state , pdf , phone_state_trans_num_dict[((int)(phone) , (int)(state))]

print '</HMMTransitionModel>'

f.close()
