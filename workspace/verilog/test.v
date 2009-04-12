`include "dds.v"

module test;
wire [7:0] out;
wire sym;
reg [3:0] init;
reg wr;
reg clk;

wire [7:0] out2;
wire sym2;
reg [3:0] init2;
reg wr2;
reg clk2;

wire [7:0] out3;
wire [7:0] out4;

dds test_dd1(out,sym,init,wr,clk);
dds test_dd2(out2,sym2,init2,wr2,clk2);


assign out3=((sym==1'b1)?((8'hff)/2-out/2):(out/2+7'b1111111));
assign out4=((sym2==1'b1)?((8'hff)/2-out2/2):(out2/2+7'b1111111));

initial
begin
	$dumpfile("test.dump");
	$dumpvars(0,clk);
	$dumpvars(1,wr);
	$dumpvars(2,init);
	$dumpvars(3,out3);
	$dumpvars(4,sym);
	$dumpvars(5,clk2);
	$dumpvars(6,wr2);
	$dumpvars(7,init2);
	$dumpvars(8,out2);
	$dumpvars(9,sym2);

	clk=1'b0;
	wr=1'b0;
	init=4'h0f;
	clk2=1'b0;
	wr2=1'b0;
	init2=4'h05;
end

always
begin
	#5 
	begin
		clk=~clk;
		clk2=~clk2;
	end
	#5
	begin
       		wr=1'b1;
		wr2=1'b1;
	end
end
endmodule
