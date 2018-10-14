target=libpanda.so
fwk_src=$(wildcard Framework/src/*.cc)
src=$(filter-out @NAMESPACE@/src/dict.cc,$(wildcard @NAMESPACE@/src/*.cc))
fwk_obj=$(patsubst Framework/src/%.cc,obj/Framework/%.o,$(fwk_src))
obj=$(patsubst @NAMESPACE@/src/%.cc,obj/@NAMESPACE@/%.o,$(src))

$(target): $(obj) $(fwk_obj) obj/@NAMESPACE@/dict.o
	g++ -std=c++17 -O2 -fPIC -shared $(shell root-config --libs) -o $@ $^

obj/@NAMESPACE@/dict.o: $(patsubst %.cc,%.h,$(subst /src/,/interface/,$(src))) @NAMESPACE@/src/LinkDef.h
	mkdir -p obj/@NAMESPACE@
	rootcling -f @NAMESPACE@/src/dict.cc $^
	g++ -std=c++17 -O2 -fPIC -c -o $@ -I$(shell root-config --incdir) -I$(shell pwd) @NAMESPACE@/src/dict.cc

obj/Framework/%.o: Framework/src/%.cc Framework/interface/%.h
	mkdir -p obj/Framework
	g++ -std=c++17 -O2 -fPIC -c -o $@ -I$(shell root-config --incdir) $<

obj/@NAMESPACE@/%.o: @NAMESPACE@/src/%.cc @NAMESPACE@/interface/%.h
	mkdir -p obj/@NAMESPACE@
	g++ -std=c++17 -O2 -fPIC -c -o $@ -I$(shell root-config --incdir) $<

.PHONY: clean

clean:
	rm -rf */src/dict.cc obj $(target)
