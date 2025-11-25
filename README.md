# ObjectStorage
The ObjectStorage class implements the following public interface:
- add(key, object)  : add object to store
- get(key) : retrieve object from store
- remove(key) : mark or remove object from store such that subsequent get(key) fails
- getAll() : return all keys of non-removed objects

<br/><br/>
Rules of completion:
- No dynamic memory allocation
- Thread-safe (i.e., N threads could be using simultaneously) 
- Non-locking