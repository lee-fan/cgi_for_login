TARGET:index.cgi GetUUID
obj=*.o
inc=-Iinclude
lib=-lsqlite3 -Llib -lcgic -lctemplate -lcJSON -lm
index.cgi:index.o common.o sqlite.o
	gcc $^ -o $@ $(inc) -luuid $(lib)
GetUUID:GetUUID.c
	gcc $^ -o $@ -luuid

clean:
	rm -rf $(obj)
cleanall:
	rm -rf $(obj) $(TARGET)
