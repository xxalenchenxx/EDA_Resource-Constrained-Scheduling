## compile:
    make clean && make

## run:
    ./mlrcs <-h/-e> <BLIF_FILE> <AND_CONSTRAINT> <OR_CONSTRAINT> <NOT_CONSTRAINT>
    example:
         ./mlrcs -h ./aoi_benchmark/aoi_sample02.blif 2 1 1
         ./mlrcs -e ./aoi_benchmark/aoi_sample02.blif 2 1 1

========================= Observations of -h and -e =========================  
In this program, list scheduling selects ready nodes based on their ID instead of following the critical path, as defined by Hu's algorithm. However, regardless of the selection strategy, certain limitations in list scheduling become apparent.
### Limitations of List Scheduling  
- List scheduling considers each type of operation independently, which can lead to suboptimal results in certain cases.
- Resource balancing across cycles is often overlooked. While specific paths in a circuit may require more resources, this scheduling strategy neglects the impact of resource-intensive operations on others, potentially delaying operations that require fewer resources. This results in increased latency.


## My observation  
We ignore balancing resource scheduling in each cycle. Although the circuit may require more resources to compute in certain paths, it overlooks that operations needing more resources can block operations that require fewer resources. This leads to the scheduling needing to spend more cycles to complete.

The following is an example:  

We set resources <AND,OR,NOT> = <2, 1, 1> because the number of AND operations is the most in this example, so we set it to 2.
There are three critical paths in my example, e.g., `{d-f-g}`,`{a,e,g}`,`{b,e,g}` need three cycles accomplish.


![alt text](image-1.png)  
test_case.blif file path: `./aoi_benchmark/test_case.blif`

.inputs x y z  
.outputs g h

```markdown
Heuristic Scheduling Result
1: {a b} {} {c} 
2: {d e} {} {} 
3: {} {f} {} 
4: {} {g} {} 
5: {} {h} {} 
LATENCY: 5
END
```


In Heuristic Scheduling of 3-5 cycles, there are only schedule OR operation,
but we can realize that `2: { de }{  }{  }` dosen't schedule any OR operation in this cycle. It wastes resource in this cycle.
We can observe that it doesn't schedule AND operation {d} to result in more latency.
Therefore, this is one of drawback of list scheduling- didn't consider other operation!!     
```markdown
ILP-based Scheduling Result
1: {b d} {} {c} 
2: {a} {f} {} 
3: {e} {h} {} 
4: {} {g} {} 
LATENCY: 4
END
```
In the ILP-based Scheduling, we can observe ILP picks AND operation {d}, then it improve other operations schedule.
As you can see, result of ILP-based Scheduling can distribute resource as many as possible in each cycle.
Even if path{d-f-g} is not a critical path, if there are many OR or NOT operations after {d}, this issue still exist.

## Conclusion  
In circuits with mixed operation types, dependencies among operations can significantly impact scheduling efficiency.

- ILP-based Scheduling provides an optimal solution by distributing resources effectively in each cycle. However, for large and complex datasets, such as `aoi_big1.blif`, ILP may require substantial computational time, making it less practical.
- Heuristic Scheduling, while less optimal, offers a reasonable solution within a shorter time frame and is more suitable for large-scale problems.
  
`Student ID: M11202158`
