target=lib/lib@NAMESPACE@.so
fwk_src=$(wildcard ../Framework/src/*.cc)
fwk_obj=$(patsubst ../Framework/src/%.cc,../Framework/obj/%.o,$(fwk_src))
fwk_inc=$(patsubst ../Framework/src/%.cc,../Framework/interface/%.h,$(fwk_src))
src=$(wildcard src/*.cc)
obj=$(patsubst src/%.cc,obj/%.o,$(src))
inc=$(patsubst src/%.cc,interface/%.h,$(src))

$(target): $(obj) $(fwk_obj) obj/dict.o
	mkdir -p lib
	g++ -std=c++17 -O2 -fPIC -shared $(shell root-config --libs) -o $@ $^

obj/dict.o: $(inc) $(fwk_inc) src/LinkDef.h
	mkdir -p obj
	rootcling -f obj/dict.cc $^
	g++ -std=c++17 -O2 -fPIC -c -o $@ -I$(shell root-config --incdir) -I$(shell pwd) obj/dict.cc
	mkdir -p lib
	mv obj/dict_rdict.pcm lib/

../Framework/obj/%.o: ../Framework/src/%.cc ../Framework/interface/%.h
	mkdir -p ../Framework/obj
	g++ -std=c++17 -O2 -fPIC -c -o $@ -I$(shell root-config --incdir) $<

obj/%.o: src/%.cc interface/%.h
	mkdir -p obj
	g++ -std=c++17 -O2 -fPIC -c -o $@ -I$(shell root-config --incdir) $<

.PHONY: clean

clean:
	rm -rf obj lib ../Framework/obj ../Framework/lib
