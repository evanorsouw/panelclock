
using System;

public class Assignment
{
    private Action _assigner;

    public Assignment(Action assigner)
    {
        _assigner = assigner;
    }
    public void Make() { _assigner(); }

}