CC=g++
# arm-linux-gnueabihf-g++

AppDependence= mongoose.c


AppLibraries=-pthread


TARGETS= LogServer


all: $(TARGETS)

$(TARGETS): %: %.cpp $(AppDependence)
	$(CC) -std=c++14 $(CFLAGS) $< $(AppDependence) -o $@ $(AppLibraries)

        
.PHONY : run
run: $(TARGETS)
	$(foreach v, $(TARGETS), ./$(v))  

.PHONY : clean
clean:
	rm -f $(TARGETS)
