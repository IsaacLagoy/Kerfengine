# Kerfengine

```bash 
find ../src -type f -exec wc -l {} + 
```

Notes about node transfer between scenes and between nodes
* Node trees must all have the same scene. If possible, remove the scene back reference entirely
* When a node is reparented or orphaned, it should keep all of its children. 
* If a node is removed from a scene, it should not be deleted and take all of its children with it
* When a node is added to a scene, all of its children should be added to the respective linked lists. When it is removed, all of the children should be removed from the linked lists.
* Nodes should be able to exist without a scene in a hierarchy as normal
* For now, copy and move operators are disabled to make implementation easier