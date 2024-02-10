library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity cmd_lut is
   port (   
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic; -- indication that new data is available
      --
      o_busy           : out std_logic; -- indication that command is in progress
      o_need_more_data : out std_logic; -- indication that more data is needed
      o_lut_waddr      : out std_logic_vector(7 downto 0);
      o_lut_wdata      : out std_logic_vector(7 downto 0);
      o_lut_wren       : out std_logic
   );
end entity;

-- command to fill the color lut with values.
-- expected byte sequence: 0x10,<lut0>,<lut1>,...,<lut255> - fill lut arbitrarily
-- expected byte sequence: 0x11 - fill lut linearly
-- expected byte sequence: 0x12 - fill lut logarithmicly
architecture rtl of cmd_lut is   

   type T_STATE is ( CMD, WRITE_LUT, FILL_LIN, FILL_LOG );  
   signal s_state     : T_STATE;
   signal s_cmd       : integer;
   signal s_idx       : integer;
   signal s_wren      : std_logic;

   type t_Data is array (0 to 255) of std_logic_vector(7 downto 0);
   constant s_lin2log : t_Data := (
      x"00", x"00", x"00", x"00", x"01", x"01", x"01", x"01",
      x"02", x"02", x"02", x"02", x"03", x"03", x"03", x"04",
      x"04", x"04", x"05", x"05", x"05", x"05", x"06", x"06",
      x"06", x"07", x"07", x"07", x"08", x"08", x"08", x"09",
      x"09", x"09", x"0a", x"0a", x"0a", x"0b", x"0b", x"0b",
      x"0c", x"0c", x"0d", x"0d", x"0d", x"0e", x"0e", x"0e",
      x"0f", x"0f", x"10", x"10", x"10", x"11", x"11", x"12",
      x"12", x"13", x"13", x"13", x"14", x"14", x"15", x"15",
      x"16", x"16", x"17", x"17", x"18", x"18", x"18", x"19",
      x"19", x"1a", x"1a", x"1b", x"1b", x"1c", x"1c", x"1d",
      x"1e", x"1e", x"1f", x"1f", x"20", x"20", x"21", x"21",
      x"22", x"22", x"23", x"24", x"24", x"25", x"25", x"26",
      x"27", x"27", x"28", x"28", x"29", x"2a", x"2a", x"2b",
      x"2c", x"2c", x"2d", x"2e", x"2e", x"2f", x"30", x"30",
      x"31", x"32", x"32", x"33", x"34", x"35", x"35", x"36",
      x"37", x"38", x"38", x"39", x"3a", x"3b", x"3c", x"3c",
      x"3d", x"3e", x"3f", x"40", x"40", x"41", x"42", x"43",
      x"44", x"45", x"46", x"47", x"47", x"48", x"49", x"4a",
      x"4b", x"4c", x"4d", x"4e", x"4f", x"50", x"51", x"52",
      x"53", x"54", x"55", x"56", x"57", x"58", x"59", x"5a",
      x"5b", x"5c", x"5e", x"5f", x"60", x"61", x"62", x"63",
      x"64", x"65", x"67", x"68", x"69", x"6a", x"6c", x"6d",
      x"6e", x"6f", x"71", x"72", x"73", x"74", x"76", x"77",
      x"78", x"7a", x"7b", x"7c", x"7e", x"7f", x"81", x"82",
      x"84", x"85", x"87", x"88", x"89", x"8b", x"8d", x"8e",
      x"90", x"91", x"93", x"94", x"96", x"98", x"99", x"9b",
      x"9d", x"9e", x"a0", x"a2", x"a3", x"a5", x"a7", x"a9",
      x"aa", x"ac", x"ae", x"b0", x"b2", x"b4", x"b5", x"b7",
      x"b9", x"bb", x"bd", x"bf", x"c1", x"c3", x"c5", x"c7",
      x"c9", x"cb", x"ce", x"d0", x"d2", x"d4", x"d6", x"d8",
      x"db", x"dd", x"df", x"e1", x"e4", x"e6", x"e8", x"eb",
      x"ed", x"f0", x"f2", x"f4", x"f7", x"f9", x"fc", x"ff"
   );

begin
   process (i_reset_n, i_clk)
   variable v_writenext : std_logic;
   begin
      if i_reset_n = '0' then
         s_state <= CMD;
         o_busy <= '0';
         o_need_more_data <= '0';         
         s_wren <= '0';
            
      elsif rising_edge(i_clk) then
                       
          if s_state = CMD then
            if i_data_rdy = '1' then
               o_busy <= '1';
               s_idx <= 0;
               case i_data is 
               when X"10" =>  -- write lut
                  o_need_more_data <= '1';
                  s_state <= WRITE_LUT;
               when X"11" =>  -- fill lut linear
                  s_state <= FILL_LIN;
               when X"12" =>  -- fill lut logarithmic
                  s_state <= FILL_LOG;
               when others =>
                  o_busy <= '0';
               end case;
            end if;
         elsif s_wren = '1' then
            s_wren <= '0';
         else
            o_lut_waddr <= std_logic_vector(to_unsigned(s_idx, o_lut_waddr'length));
            v_writenext := '0';
            
            case s_state is             
            when WRITE_LUT =>
               if i_data_rdy = '1' then
                  o_lut_wdata <= i_data;
                  v_writenext := '1';
               end if;             
            when FILL_LIN =>
               o_lut_wdata <= std_logic_vector(to_unsigned(s_idx, o_lut_wdata'length));
               v_writenext := '1';
            when FILL_LOG =>    
               o_lut_wdata <= s_lin2log(s_idx);               
               v_writenext := '1';
            when others =>           
            end case;
            
            if v_writenext = '1' then
               s_wren <= '1';
               s_idx <= s_idx + 1;
               if s_idx = 255 then
                  s_state <= CMD;
                  o_busy <= '0';
                  o_need_more_data <= '0';
               end if;
            end if;            
         end if;
         o_lut_wren <= s_wren;
      end if;      
     
   end process;   
end architecture;
                   