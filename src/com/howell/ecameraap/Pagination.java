package com.howell.ecameraap;
/**
 * @author 霍之昊 
 *
 * 类说明
 */
public class Pagination {
	//需要用户填写
    public int page_size; //每页多少条记录，0:无分页
    public int page_no; //页号, from 0

    //返回
    public int total_size; //总共多少条记录
    public int cur_size; //当前页多少条记录
    public int page_count; //总共多少页
	public Pagination(int page_size, int page_no, int total_size, int cur_size,
			int page_count) {
		super();
		this.page_size = page_size;
		this.page_no = page_no;
		this.total_size = total_size;
		this.cur_size = cur_size;
		this.page_count = page_count;
	}
	public Pagination() {
		super();
	}
	public Pagination(int page_size, int page_no) {
		super();
		this.page_size = page_size;
		this.page_no = page_no;
	}
	@Override
	public String toString() {
		return "Pagination [page_size=" + page_size + ", page_no=" + page_no
				+ ", total_size=" + total_size + ", cur_size=" + cur_size
				+ ", page_count=" + page_count + "]";
	}
    
    
}
