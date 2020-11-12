

var task = new Task(init, this);
task.schedule(100);

function init()
{

	var b = this.patcher.getnamed("q_tab");
    if (b == null)
       {
               this.patcher.message("script", "newobject", "newobj", "@text","p ?", "@varname", "q_tab", "@patching_rect", 205, 205, 50, 20);
               var q=this.patcher.getnamed("q_tab");
               q.subpatcher().message("wclose");
               q.message("showontab", 1);
       }
}

function resize(x, y)
{
	if(x==null)
	{
		this.patcher.wind.size=[853, 725];
	}
	else
	{
		this.patcher.wind.size= [x, y];
	}
}
