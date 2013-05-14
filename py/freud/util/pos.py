import numpy
import re

class file:

    def __init__(self, filename):
        self.fname = filename;
        print("opening {0} for reading".format(self.fname));
        self.isInitialized = False;
        self.initialize();
        print("read file successfully");

# Maybe I should use the tell method to mark the positions of shit...

    def initialize(self):
        # Counts number of data, done, and eof tags
        self.ndata = 0;
        self.ndone = 0;
        self.nbox = 0;
        self.neof = 0;
        
        # Creates dictionaries for the data dims at each frame
        # The number of data dims at each frame
        # Number of data points at each frame
        self.data_dims = [];
        self.n_data_dims = [];
        self.n_data_points = [];
        
        self.box_dims = [];
        self.n_box_dims = [];
        self.n_def_count = [];
        # These dictionaries store the position of the reader at each point
        # Will be used for the grab commands
        # The tell values will place you directly at the beginning of the line following the data line
        self.data_tell = [];
        self.done_tell = [];
        self.box_tell = [];
        self.eof_tell = [];
        
        #Dictionary that will hold the data, position numpy arrays at each frame
        #self.data = {};
        #self.position = {};

        self.initData();
        self.initBox();

        self.ndata_frames = self.ndata
        self.nbox_frames = self.nbox

        if not self.ndata_frames == self.nbox_frames:
            # This doesn't do anything as some pos files will not have both
            print("not all frames have matching data and box")
            print("total of {0} data and {1} box frame detected")

        print("initial read complete")
        print("ndata = {0} nbox = {1}".format(self.ndata_frames, self.nbox_frames))
        self.isInitialized = True;

    def initData(self):
        n_data_dims = []
        data_dims = []
        data_tell = []
        ndata = 0
        done_tell = [];
        ndone = 0;
        f = open(self.fname, "r");
        line = f.readline();
        while line:
            # Need to count number of data lines for numpy array
            if re.match('#\[data\]', line):
                observables = re.split('\s+', line)[1:-1];
                n_data_dims.append(len(observables));
                data_dims.append(observables);
                data_tell.append(f.tell());
                ndata += 1;
            elif re.match('#\[done\]', line):
                done_tell.append(f.tell());
                ndone += 1;
            line = f.readline();
        f.close()
        if not ndata == ndone:
            print("mismatch in data = {0} and done = {1}; must eliminate bad data sets".format(ndata, ndone))
            i = 0;
            while not ndata == ndone:
                # print(i)
                # print("ndata = {0} and ndone = {1}".format(ndata, ndone))
                curr_data = data_tell[i]
                curr_done = done_tell[i]
                next_data = data_tell[i + 1]
                # print(curr_data)
                # print(curr_done)
                # print(next_data)
                isDataLTDone = (curr_data < curr_done)
                isNextGTDone = (next_data > curr_done)
                # print(isDataLTDone)
                # print(isNextGTDone)
                if not (isDataLTDone and isNextGTDone):
                    # print("removing data")
                    data_tell.pop(i)
                    n_data_dims.pop(i)
                    data_dims.pop(i)
                    ndata -= 1
                    i = 0;
                else:
                    i += 1
                if i >= ndone:
                    # print("reached end with extra data")
                    #for j in range(i, ndata):
                    while i < ndata:
                        # print("removing data")
                        # print(i)
                        data_tell.pop(i)
                        n_data_dims.pop(i)
                        data_dims.pop(i)
                        ndata -= 1
            # print("ndata = {0} ndone = {1}".format(ndata, ndone))
        self.n_data_dims = n_data_dims
        self.data_dims = data_dims
        self.data_tell = data_tell
        self.ndata = ndata
        self.done_tell = done_tell
        self.ndone = ndone

    def initBox(self):
        n_box_dims = []
        box_dims = []
        box_tell = []
        nbox = 0
        def_count = 0;
        n_def_count = []
        n_def_dims = []
        def_dims = []
        def_tell = []
        eof_tell = [];
        neof = 0;
        f = open(self.fname, "r");
        line = f.readline();
        while line:
            if re.match('^box', line):
                box_dims = re.split('\s+', line)[1:-1];
                n_box_dims.append(len(box_dims));
                box_dims.append(box_dims);
                box_tell.append(f.tell());
                nbox += 1;
                def_count = 0;
                tmp_def_tell = f.tell();
            # The defs will be broken currently if the file ends before eof...
            # Should this be somewhere else???
            elif re.match('^def', line):
                defs = re.split('\s+', line)[1:-1];
                n_def_dims.append(len(defs))
                def_dims.append(defs)
                def_count += 1;
                tmp_def_tell = f.tell();
            elif re.match('^eof', line):
                n_def_count.append(def_count);
                def_tell.append(tmp_def_tell)
                eof_tell.append(f.tell());
                neof += 1;
            line = f.readline();
        f.close()
        if not nbox == neof:
            print("mismatch in box = {0} and eof = {1}; must eliminate bad data sets".format(nbox, neof))
            i = 0;
            while not nbox == neof:
                curr_box = box_tell[i]
                curr_eof = eof_tell[i]
                next_box = box_tell[i + 1]
                isBoxLTEof = (curr_box < curr_eof)
                isNextGTEof = (next_box > curr_eof)
                if not (isBoxLTEof and isNextGTEof):
                    box_tell.pop(i)
                    n_box_dims.pop(i)
                    box_dims.pop(i)
                    nbox -= 1
                    i = 0;
                else:
                    i += 1
                if i >= neof:
                    while i < nbox:
                        box_tell.pop(i)
                        n_box_dims.pop(i)
                        box_dims.pop(i)
                        nbox -= 1
        self.n_box_dims = n_box_dims
        self.box_dims = box_dims
        self.box_tell = box_tell
        self.n_def_count = n_def_count
        self.n_def_dims = n_def_dims
        self.def_dims = def_dims
        self.nbox = nbox
        self.def_tell = def_tell
        self.eof_tell = eof_tell
        self.neof = neof
    
    def grabData(self):
        self.n_data_points = numpy.zeros(self.ndata, dtype=numpy.int32)
        # print(self.n_data_points.shape)
        f = open(self.fname);
        for i in range(self.ndata):
            # print("starting frame {0}".format(i))
            f.seek(self.data_tell[i]);
            data_cnt = 0;
            line = f.readline();
            while not re.match('#\[done\]', line):
                data_cnt += 1;
                line = f.readline();
            self.n_data_points[i] = data_cnt;
        f.close();
        n_data_points = 0;
        for i in self.n_data_points:
            if i > n_data_points:
                n_data_points = i
        n_data_dims = 0;
        for i in self.n_data_dims:
            if i > n_data_dims:
                n_data_dims = i
        self.data = numpy.zeros(shape = (self.ndata, n_data_points, n_data_dims), dtype=numpy.float32);
        f = open(self.fname)
        for i in range(self.ndata):
            # print("starting frame {0}".format(i))
            f.seek(self.data_tell[i])
            for j in range(self.n_data_points[i]):
                line = f.readline();
                tmp_data = re.split('\s+', line);
                for k in range(self.n_data_dims[i]):
                    # Need the raw data here...
                    self.data[i][j][k] = tmp_data[k]

    def grabBox(self):
        # We have a problem in which the box line is complex and shit and needs extra parsing
        # Simple numpy array like for data will not cut it. 
        self.n_box_points = numpy.zeros(self.nbox, dtype=numpy.int32)
        f = open(self.fname);
        for i in range(self.nbox):
            # print("starting frame {0}".format(i))
            # Redundant def count...
            f.seek(self.box_tell[i]);
            data_cnt = 0;
            line = f.readline();
            while not re.match('^eof', line):
                if not re.match('^def', line):
                    data_cnt += 1;
                line = f.readline();
            self.n_box_points[i] = data_cnt;
        f.close();
        n_box_points = 0;
        for i in self.n_box_points:
            if i > n_box_points:
                n_box_points = i
        n_box_dims = 0;
        for i in self.n_box_dims:
            if i > n_box_dims:
                n_box_dims = i
        self.box = numpy.zeros(shape = (self.nbox, n_box_points, n_box_dims), dtype=numpy.float32);
        f = open(self.fname)
        for i in range(self.ndata):
            f.seek(self.def_tell[i])
            for j in range(self.n_box_points[i]):
                # Near as I can tell, my def_tell works!
                line = f.readline();
                tmp_box = re.split('\s+', line);
                # Need a super awesome function call here
                self.boxParse(tmp_box)
                # for k in range(self.n_box_dims[i]):
                #     # Need the raw data here...
                #     self.box[i][j][k] = tmp_box[k]
    def boxParse(self, box_string):
        print(box_string)